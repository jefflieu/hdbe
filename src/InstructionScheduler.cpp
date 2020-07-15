/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include "llvm/ADT/Twine.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstIterator.h"

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
  LOG_S(INFO) << "Function: " << irFunction->getName() << "\n";


  LOG_S(IS_DBG) << "Assigning birth time to global variables and arguments \n";
  //Assign birth time for Global Variable
  for(auto gv_i = M.global_begin(), gv_end = M.global_end(); gv_i != gv_end; ++gv_i)
    {
      LOG_S(IS_DBG) << *gv_i << "\n" ;
      auto ret = CDI_h->addValueInfo(&*gv_i);
      ret.first->second.setBirthTime(0.0, 0.0);
    }

  //Assign valid_time for Arguments:
  for(auto arg_i = F.arg_begin(), arg_end = F.arg_end(); arg_i != arg_end; arg_i ++ )
    {
      LOG_S(IS_DBG) << *arg_i << "\n";
      auto ret = CDI_h->addValueInfo(&*arg_i);
      ret.first->second.setBirthTime(0.0, 0.0);
    }

  //Assign valid time for entry Block
  auto entryBlkInfo = CDI_h->addValueInfo(&entryBlock);
  entryBlkInfo.first->second.setBirthTime(0.0, 0.0);


  LOG(IS_DBG, "Collecting instructions for scheduling");
  std::list<Instruction * > instructions;
  std::map<BasicBlock *, int > bbInstrCountMap;

  for(auto bb_i = F.begin(), bb_end = F.end(); bb_i != bb_end ; ++bb_i)
  {
    LOG_S(IS_DBG) << " Block : " << bb_i->getName() << "\n";

    // Collecting all instructions in the basic block
    for(auto ins_i = bb_i->begin(), ins_end = bb_i->end(); ins_i != ins_end; ++ ins_i)
    {

      instructions.push_back(&*ins_i);
      LOG_S(IS_DBG) << "Instruction: " << &*ins_i << "  " << *ins_i << "\n";

      //Assigning Valid time for constants
      for(const llvm::Use &use : ins_i->operands())
      {
        llvm::Value* val = use.get();
        if (llvm::Constant::classof(val))
        {
          auto ret = CDI_h->addValueInfo(&*val);
          ret.first->second.setBirthTime(0.0, 0.0);
        }
      }
    }
  }

  HWD.initializeHWResources();
  
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
        Calculate the ealiest time an instruction can be scheduled
        Round it up to the step if values have been valid in previous steps
      */
      dependency_valid = std::max<float>(getDependencyValidTime(I), step);


      /*
        Request information from Hardware Description/Manager block
        The combination of latency and valid time will ultimately decide whether the instruction can be executed
        Later on, this HWD will also bind the instruction to a particular hardware engine
      */
      HardwareDescription::ExecutionInfo EI = HWD.requestToSchedule(I, dependency_valid);

      LOG_S(IS_DBG + 1) << "Dependency valid time: " << dependency_valid << ", value valid time: " << EI.valid << "\n";

      bool instructionSchedulable = EI.hw_available && (dependency_valid < (step + 1.0) && (EI.valid < step + 1.0 || EI.latency >= 1.0));

      //Additional check for terminator
      if (I->isTerminator())
        instructionSchedulable = instructionSchedulable && isBranchSchedulable(I, step);
      else if (isMemoryInstruction(I))
        instructionSchedulable = instructionSchedulable && isMemoryInstSchedulable(I, step);

      if  ( instructionSchedulable ) {

        LOG_S(INFO) << "Instruction: " << *I << " has been scheduled \n";
        LOG_S(IS_DBG + 2) << "Valid time " << EI.valid << "\n";

        HWD.updateHWResources(I);

        if (I->getOpcode()==llvm::Instruction::Ret)
          state.termInstruction = I;
        else
          state.instructionList.push_back(I);


        //Give birth time to OUTPUTs
        auto producedValues = getInstructionOutputs(I);
        for(auto val : producedValues) {
          if (BasicBlock::classof(val) && !isBasicBlockSchedulable(static_cast<BasicBlock*>(val))) continue;
          auto ret = CDI_h->addValueInfo(val);
          ret.first->second.setBirthTime(step, EI.valid);
          LOG_S(IS_DBG) << "Set birth time of " << *val << "\n";
          LOG_S(IS_DBG) << " >>> " << step << " " << EI.valid << "\n";
        }

        //When a value is fed back in a loop, we artificially add use time = valid time
        // if (isBackValue(static_cast<Value*>(I))) {
        //   LOG_S(INFO) << *I << "is a back value\n";
        //   VIM[I].addUseTime(floor(EI.valid));
        // }
        if (I->isTerminator())
          updateBackValueUseTime(I);


        //The instruction has been scheduled, we update the operands useTime
        auto usedValues = getInstructionInputs(I);
        for(auto val : usedValues)
        {
          VIM[val].addUseTime(step);
        }

        //Finally remove instruction from work list
        list_i = instructions.erase(list_i);

      } else {

        //Advance iterator to consider the next instruction in the queue
        ++ list_i;

      }

    }

    //After considering all instructions in the queue, we advance the time step
    step ++ ;
    HWD.nextStep();

    //For debugging, if we keep iterating for 20 steps and we still can't finish
    if (step >= MAX_STEP) dumpInstructions(instructions);
    ASSERT(step < MAX_STEP, "Something wrong in scheduling process") ;
  }


  /*
    All instructions has been scheduled
  */

  //Update the "last" flag
  stateList.back().setLast(true);


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
      float operand_valid = VIM[val].valid.time;
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
          //LOG_S(INFO) << bb << " vs " << static_cast<Instruction*>(value)->getParent() << "\n";
          if (value == v)
          {
            HdlCFGEdge &e = CDI_h->findCFGEdge(bb, phi->getParent());
            is_fed_back = e.isBackEdge();
            //Final condition check if the value is BackValue 
            //ASSERT(((!is_fed_back) || bb == (static_cast<Instruction*>(value)->getParent())), "Not supported control structure\n"); 
          }
        }
    }
  }

  return is_fed_back;
}

float InstructionScheduler::getDependencyValidTime(Instruction* instr)
{
  auto VIM = CDI_h->valueInfoMap;
  float dependency_valid = 0;
  auto usedValues = getInstructionInputs(instr);
  for(auto val : usedValues)
  {
    if (VIM.count(val) == 0) {
      dependency_valid = 1.0e6;
      LOG_S(IS_DBG + 2) << getBriefInfo(val) << "Not found \n";
      break;
    }
    float operand_valid = VIM[val].valid.time;
    dependency_valid = std::max<float>(dependency_valid, operand_valid);
  }

  return dependency_valid;
}

bool InstructionScheduler::isBranchSchedulable(Instruction * brInst, float current_step)
{
  auto &VIM               = CDI_h->valueInfoMap;
  float  dependency_valid = current_step;
  BasicBlock* parentBlock = brInst->getParent();
  auto &LI                = CDI_h->getLoopInfo();
  //Additional condition for branch instruction
  assert(brInst->isTerminator());

  if (brInst->getOpcode() == llvm::Instruction::Ret)
  {
    //For a return instruction, all other values must be valid before the Return instruction is scheduled.
    for (auto iter = inst_begin(CDI_h->irFunction), E = inst_end(CDI_h->irFunction); iter != E; ++iter)
    {
      Instruction & I = *iter;
      if ((&I) == brInst) continue;
      if (VIM.count(&I) == 0) {
        dependency_valid = 1.0e6;
        LOG_S(IS_DBG + 2) << getBriefInfo(&I) << "Not found \n";
        break;
      }
      dependency_valid = std::max<float>(dependency_valid, VIM[&I].valid.time);
    }
  } else {
     //This loop to ensure that all instruction in the same BB have been schduled
     for (Instruction &I : parentBlock->getInstList())
     {
       if ((&I) == brInst) continue;
       if (VIM.count(&I) == 0) {
         dependency_valid = 1.0e6;
         LOG_S(IS_DBG + 2) << getBriefInfo(&I) << "Not found \n";
         break;
       }
       dependency_valid = std::max<float>(dependency_valid, VIM[&I].schedule.time);
     }

     //This loop iterates the successor of the branch instruction 
     //In each successor, it looks at the PHI instruction and looks for the Value associated with parentBlock
     //If the transition is a backedge: this Value has to be valid after the transition (1 cycle)
     //If the transition is not a backedge, this value has to be valid. 
     //The condition is to ensure that all PHI instructions are executed at the first cycle that the SuccessorBlock is active
     for(unsigned s = 0; s < brInst->getNumSuccessors(); s++)
     {
        BasicBlock * succ = brInst->getSuccessor(s);
        for(Instruction &I : succ->getInstList())
        {
          llvm::PHINode * phiInst;
          if (phiInst = llvm::dyn_cast<llvm::PHINode>(&I)) {
            for(unsigned i = 0; i < phiInst->getNumIncomingValues(); i++)
            {        
              if (phiInst->getIncomingBlock(i) == parentBlock)
              {
                Value* val = phiInst->getIncomingValue(i);
                if (VIM.count(val) == 0) {
                  dependency_valid = 1.0e6;
                  LOG_S(IS_DBG + 2) << getBriefInfo(val) << "Not found \n";
                  break;
                } else {
                  bool isBackEdge = (LI.isLoopHeader(succ) && (LI.getLoopFor(parentBlock) == LI.getLoopFor(succ)));
                  auto valid = isBackEdge?(ceil(VIM[val].valid.time) - 1.0):VIM[val].valid.time;
                  dependency_valid = std::max<float>(dependency_valid, valid);
                  LOG_S(INFO) << "Back value " << val->getName() << "\n";
                  LOG_S(INFO) << "Valid time " << VIM[val].valid.time << " " << valid << "\n";
                }
              }
            }
          }
        }
     }
  }
  // } else {
  //   /* This whole block of code is concerned of a branch instruction that loops back 
  //   to the loop header. So it traces the back value associated with the backedge 
  //   and decide when the backedge can be done */ 
  //   LoopInfo &LI = CDI_h->LI;
  //   llvm::Loop *loop = LI.getLoopFor(parentBlock);
  //   bool isLoopLatch = false;
  //   if (loop != nullptr) isLoopLatch = loop->isLoopLatch(parentBlock);
  //   if (isLoopLatch) {
  //     BasicBlock* header  = loop->getHeader();
  //     //Iterate Phis of header 
  //     for(Instruction &I : header->getInstList())
  //     {
  //       llvm::PHINode * phiInst;
  //       if (phiInst = llvm::dyn_cast<llvm::PHINode>(&I)) {
  //         for(unsigned i = 0; i < phiInst->getNumIncomingValues(); i++)
  //         {        
  //           //For other branch instruction, we only care about the Loopback value (a value is read on next loop iteration)
  //           //The branch instruction can only be scheduled if the all back-values in the basicblock is valid
  //           if (phiInst->getIncomingBlock(i) == parentBlock)
  //           {
  //             Value* val = phiInst->getIncomingValue(i);
  //             if (VIM.count(val) == 0) {
  //               dependency_valid = 1.0e6;
  //               LOG_S(IS_DBG + 2) << getBriefInfo(val) << "Not found \n";
  //               break;
  //             } else {
  //               auto valid = ceil(VIM[val].valid.time) - 1.0;
  //               dependency_valid = std::max<float>(dependency_valid, valid);
  //               LOG_S(INFO) << "Back value " << val->getName() << "\n";
  //               LOG_S(INFO) << "Valid time " << VIM[val].valid.time << " " << valid << "\n";
  //             }
  //           }
  //         }
  //       }
  //     }
  //   }
  // }
  return (dependency_valid < (current_step + 1.0));
}

/*
  A Basicblock is schedulable when all predecessors' terminators have been scheduled
*/
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


void InstructionScheduler::updateBackValueUseTime(Instruction* brInst)
{
  BasicBlock* parentBlock = brInst->getParent();
  auto &VIM               = CDI_h->valueInfoMap;
  auto &LI                = CDI_h->getLoopInfo();
  auto loops              = LI.getLoopsInPreorder();
  LOG_S(INFO) << "Loop cnt: " << loops.size() << "\n";
  llvm::Loop *loop        = LI.getLoopFor(parentBlock);
  bool isLoopLatch        = false;
  isLoopLatch = (loop && loop->isLoopLatch(parentBlock)); 
  if (isLoopLatch) {
    BasicBlock* header  = loop->getHeader();
    //Iterate Phis of header 
    for(Instruction &I : header->getInstList())
    {
      llvm::PHINode * phiInst;
      if (phiInst = llvm::dyn_cast<llvm::PHINode>(&I)) {
        for(unsigned i = 0; i < phiInst->getNumIncomingValues(); i++)
        {        
          //For other branch instruction, we only care about the Loopback value (a value is read on next loop iteration)
          //The branch instruction can only be scheduled if the all back-values in the basicblock is valid
          if (phiInst->getIncomingBlock(i) == parentBlock)
          {
            Value* val = phiInst->getIncomingValue(i);
            assert(VIM.count(val) > 0);
            auto use_time = floor(VIM[(Value*)brInst].schedule.time) + 1.0;
            VIM[val].addUseTime(use_time);
            LOG_S(INFO) << "Updated use time of back value: " << val->getName() << " --> " << use_time << "\n";
          }
        }
      }
    }
  }

  // for(Instruction  &I : parentBlock->getInstList())
  // {
  //   if ((&I) == brInst) continue;
  //   if (isBackValue((Value*)&I))
  //   {
  //     assert(VIM.count(&I) > 0);
  //     auto use_time = floor(VIM[(Value*)brInst].schedule.time) + 1.0;
  //     VIM[&I].addUseTime(use_time);
  //     LOG_S(INFO) << "Updated use time of back value: " << I.getName() << " --> " << use_time << "\n";
  //   }
  // }
}

bool InstructionScheduler::isMemoryInstSchedulable(Instruction * memInstr, float current_step)
{
  auto &VIM               = CDI_h->valueInfoMap;
  auto &DM                = CDI_h->dependencyMap;
  float  dependency_valid = current_step;
  //Additional condition for branch instruction
  assert("Instruction is not memory instruction" && isMemoryInstruction(memInstr));

  auto & depInfo = DM[static_cast<Value*>(memInstr)];

  for(auto &item : depInfo.getDependenceMap())
  {
    if (item.second == DependenceType::NoDependency) continue;
    if (VIM.count(item.first) == 0) {
      dependency_valid = 1.0e6;
      LOG_S(IS_DBG + 2) << getBriefInfo(item.first) << "Not found \n";
      break;
    }
    if (item.second == DependenceType::Valid)
      dependency_valid = std::max<float>(dependency_valid, VIM[item.first].valid.time);
    else 
      dependency_valid = std::max<float>(dependency_valid, VIM[item.first].schedule.time);

  }

  return (dependency_valid < (current_step + 1.0));
}


