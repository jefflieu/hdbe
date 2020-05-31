#include "llvm/ADT/Twine.h"
#include "llvm/IR/InstrTypes.h"

#include "ValueLifeInfo.hpp"
#include "logging/logger.hpp"
#include "InstructionScheduler.hpp"

#ifndef  IS_DBG 
#define  IS_DBG 9
#endif  
#define MAX_STEP 20 

using namespace std;
using namespace hdbe;


void InstructionScheduler::schedule(BasicBlock * irBasicBlock){};

void InstructionScheduler::schedule() {
 this->schedule(CDI_h->irFunction);
}

void InstructionScheduler::schedule(Function * irFunction) 
{
  /* 
    Map instructions to SPACE and TIME        
  */

  auto &M              = *(CDI_h->irModule);
  auto &F              = *irFunction;
  auto &HWD            = CDI_h->HWD;
  auto &stateList      = CDI_h->stateList;
  auto &VIM            = CDI_h->valueInfoMap;
  auto &entryBlock     = *(F.begin());
  auto &variableList   = CDI_h->variableList;
  
  LOG_START(INFO);
  LOG_S(INFO) << irFunction->getName() << "\n"; 
        
    
  LOG_S(IS_DBG) << "Assigning birth time to global variables and arguments \n";      
  //Assign birth time for Global Variable 
  for(auto gv_i = M.global_begin(), gv_end = M.global_end(); gv_i != gv_end; ++gv_i)
    {
      LOG_S(IS_DBG) << *gv_i << "\n" ;
      auto ret = CDI_h->addValueInfo(&*gv_i);
      ret.first->second.setBirthTime(nullptr, 0.0);
    }

  //Assign valid_time for Arguments:   
  for(auto arg_i = F.arg_begin(), arg_end = F.arg_end(); arg_i != arg_end; arg_i ++ )
    {
      LOG_S(IS_DBG) << *arg_i << "\n";
      auto ret = CDI_h->addValueInfo(&*arg_i);
      ret.first->second.setBirthTime(nullptr, 0.0);     
    }

  //Assign valid time for entry Block 
  auto entryBlkInfo = CDI_h->addValueInfo(&entryBlock);
  entryBlkInfo.first->second.setBirthTime(nullptr, 0.0);  


  LOG(IS_DBG, "Collecting instructions for scheduling");
  std::list<Instruction * > instructions;                      
  std::map<BasicBlock *, int > bbInstrCountMap;                      

  for(auto bb_i = F.begin(), bb_end = F.end(); bb_i != bb_end ; ++bb_i)
  {
    LOG_S(IS_DBG) << " Block : " << bb_i->getName() << "\n";
    bbInstrCountMap[&*bb_i] = 0;
    
    // Collecting all instructions in the basic block 
    for(auto ins_i = bb_i->begin(), ins_end = bb_i->end(); ins_i != ins_end; ++ ins_i)
    {

      instructions.push_back(&*ins_i);
      bbInstrCountMap[&*bb_i]++;      
      LOG_S(IS_DBG) << "Instruction: " << &*ins_i << "  " << *ins_i << "\n";   
      
      //Assigning Valid time for constants
      for(const llvm::Use &use : ins_i->operands())
      {
        llvm::Value* val = use.get();
        if (llvm::Constant::classof(val))
        {
          auto ret = CDI_h->addValueInfo(&*val);
          ret.first->second.setBirthTime(nullptr, 0.0);  
        }
      }                        
    }
  }
  
    //Scheduling 
  LOG(IS_DBG, "Start scheduling");

  uint32_t step = 0;     
  
  //Keep doing the scheduling until the list of instructions is empty
  while( ! instructions.empty()) 
  {
    //Everytime we are here, we add a new time step
    stateList.push_back(HdlState(step));
    HdlState & state  = stateList.back(); 

    /* 
      Iterate the list and see if the instruction can be scheduled  
      Instruction can be scheduled when all the inputs are valid 
      Note that the inputs of an instruction is a little more than the number of operands
      For the purpose of generating the HDL, the control information (the basicblock) is part of the input 
      The basicblock itself is the OUTPUT of a branch instruction, not the input
      We change the interpretation of inputs/outputs a bit to suit our purpose 
      Essentially, there's not "branch" in our generated HDL but a flatten list of instructions to be executed. 
      But the execution of the instruction is guarded by "BasicBlock" valid bit
    */
    for (auto list_i = instructions.begin(), list_end = instructions.end(); list_i != list_end;)
    {
      Instruction* I = *list_i;
      //Get Operands           
      float dependency_valid = step;
      LOG_S(IS_DBG + 1) << "Considering " << *I << " with opcode " << I->getOpcodeName() << "\n";

      /*
        getInstructionInputs and getInstructionOutputs are implemented in the IRUtil block 
        These functions reinterpret what INPUT and OUTPUT of an instruction means
        For example, BasicBlock is output of a branch function and Input to normal instructions. 
        An instruction can only be scheduled when the parent basicblock has been "scheduled"
      */
      auto usedValues = getInstructionInputs(I);
      for(auto val : usedValues)
      {
        if (VIM.count(val) == 0) {
          dependency_valid = 1.0e6; 
          LOG_S(IS_DBG + 2) << getBriefInfo(val) << "Not found \n"; 
          break;
        }
        float operand_valid = VIM[val].birthTime.time;
        dependency_valid = std::max<float>(dependency_valid, operand_valid); 
      }

      /*
        Request information from Hardware Description/Manager block
        The combination of latency and valid time will ultimately decide whether the instruction can be executed 
        Later on, this HWD will also bind the instruction to a particular hardware engine
      */
      HardwareDescription::ExecutionInfo EI = HWD.requestToSchedule(I, dependency_valid);
      //float latency    = HWD.getLatency(I);
      //float valid_time = HWD.getValidTime(I, dependency_valid);
      float latency = EI.latency;
      float valid_time = EI.valid; 

      LOG_S(IS_DBG + 1) << "Dependency valid time: " << dependency_valid << ", value valid time: " << valid_time << "\n";

      //Branch Instruction has to be the last one to be scheduled in a basicblock 
      bool branchInstrCheck = true;
      if (I->getOpcode() == llvm::Instruction::Ret || 
            I->getOpcode() == llvm::Instruction::Br  || 
              I->getOpcode() == llvm::Instruction::Switch) {
        branchInstrCheck = (bbInstrCountMap[I->getParent()] == 1) && isBranchSchedulable(I, step);
        LOG_S(IS_DBG + 1) << "Checking branch instruction " << branchInstrCheck << "\n";
      }

      //Ok to schedule 
      if (( (valid_time >= 0 && valid_time < (step + 1.0)) || (latency >= 1.0 && dependency_valid <(step+1.0))) && branchInstrCheck) {


        LOG_S(IS_DBG + 1) << "Instruction: " << *I << " has been scheduled \n"; 
        LOG_S(IS_DBG + 2) << "Valid time " << valid_time << "\n";            

        if (I->getOpcode()==llvm::Instruction::Ret)
          state.termInstruction = I;
        else 
          state.instructionList.push_back(I);  


        //Give birth time to OUTPUTs
        auto producedValues = getInstructionOutputs(I);
        for(auto val : producedValues) {
          if (BasicBlock::classof(val) && !isBasicBlockSchedulable(static_cast<BasicBlock*>(val))) continue;
          auto ret = CDI_h->addValueInfo(val);
          ret.first->second.setBirthTime(&state, valid_time);
        }

        if (isBackValue(static_cast<Value*>(I))) {
          LOG_S(INFO) << *I << "is a back value\n";
          VIM[I].addUseTime(&state, step);
        }

        
        //The instruction has been scheduled, we update the operands useTime 
        for(auto val : usedValues)
        {
          VIM[val].addUseTime(&state, step); 
        }

        //Finally erase the item 
        list_i = instructions.erase(list_i);
        //Reduce the outstanding instruction count of the BasicBlock
        bbInstrCountMap[I->getParent()]--;

      } else {

        //Advance iterator to consider the next instruction in the queue
        ++ list_i;

      }

    }

    //After considering all instructions in the queue, we advance the time step   
    step ++ ; 
    
    //For debugging, if we keep iterating for 20 steps and we still can't finish
    if (step >= MAX_STEP) dumpInstructions(instructions);    
    ASSERT(step < MAX_STEP, "Something wrong in scheduling process") ;
  }


  /* 
    All instructions has been scheduled
  */

  //Update the "last" flag 
  stateList.back().setLast(true);

  //Update the values that has no state bound to it 
  for(auto map_i = VIM.begin(), map_end = VIM.end(); map_i!=map_end; ++ map_i)
  {
    if (! map_i->second.birthTime.state) 
      {
        map_i->second.setBirthTime(&stateList.front(),0.0);
      } 
    LOG_S(IS_DBG+1) << *(map_i->first) << " -- " << " Use time list " << map_i->second.useTimeList.size() << " \n"  ;
  }

  //Remove unused variables 
  for(auto var = variableList.begin(); var!=variableList.end();)
  {
    if (VIM[var->getIrValue()].useTimeList.empty()) {
      LOG_S(WARN) << "Removing unused variable "<<var->getIrValue()->getName() << "\n";
      var = variableList.erase(var);
    } else {
      ++var;
    }
  } 

  LOG_DONE(INFO);
}

void InstructionScheduler::dumpInstructions(std::list<Instruction*> &instList)
{
  auto &VIM = CDI_h->valueInfoMap;
  for(auto I : instList)
  {
    LOG_S(IS_DBG) << *I << "\n";
    auto usedValues = getInstructionInputs(I);
    LOG_S(IS_DBG) << "Dependency:\n";
    for(auto val : usedValues)
    {
      LOG_S(IS_DBG + 1) << *val << "\n";
      if (VIM.count(val) == 0) {        
        LOG_S(IS_DBG + 1) << "Not found \n"; 
        break;
      }
      float operand_valid = VIM[val].birthTime.time;
      LOG_S(IS_DBG + 1) << "Valid time: " << operand_valid << "\n";
    }
  }
}

bool InstructionScheduler::isBackValue(Value* v)
{
  //A value is a backvalue if it is used by a PHI node 
  //And the incoming edge is a back-edge 
  bool is_fed_back = false;
  if (!llvm::Instruction::classof(v)) return is_fed_back;
  for (User* user : v->users())
  {
    if (llvm::PHINode::classof(user))
    {
      auto phi = static_cast<llvm::PHINode*>(user);
      for(int i = 0; i<phi->getNumIncomingValues(); i++)
        { 
          BasicBlock* bb = phi->getIncomingBlock(i);
          Value* value = phi->getIncomingValue(i);          
          //if (value == v && bb == (static_cast<llvm::Instruction*>(v))->getParent()) is_fed_back = true;
          if (value == v) 
          {
            HdlCFGEdge &e = CDI_h->findCFGEdge(bb, phi->getParent());
            is_fed_back = e.isBackEdge();
          }
        }
    }
  }
  return is_fed_back;
}

bool InstructionScheduler::isBranchSchedulable(Instruction * brInst, float current_step)
{
  auto &M              = *(CDI_h->irModule);
  auto &VIM            = CDI_h->valueInfoMap;
  float  dependency_valid = 0;
  //All values produced by instrution in the basicblock must be valid 
  BasicBlock* parentBlock     = brInst->getParent();

  for(Instruction  &I : parentBlock->getInstList())
  {
    if ((&I) == brInst) continue;
    if (VIM.count(&I) == 0) {
      dependency_valid = 1.0e6;                  
      LOG_S(IS_DBG + 2) << getBriefInfo(&I) << "Not found \n"; 
      break;
    }
    dependency_valid = std::max<float>(dependency_valid, VIM[&I].birthTime.time); 
  }
  return (dependency_valid < (current_step + 1.0));
}

bool InstructionScheduler::isBasicBlockSchedulable(BasicBlock * bb)
{
  auto &M              = *(CDI_h->irModule);
  auto &VIM            = CDI_h->valueInfoMap;
  for(BasicBlock* pred : predecessors(bb))
  {
    Instruction* term = pred->getTerminator();
    assert(term);
    if (isBackEdge(pred, bb)) continue;
    if (VIM.count(term) == 0) return false;//All predecessor terminator has been scheduled
  }
  return true;
}





