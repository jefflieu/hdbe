#include "llvm/ADT/Twine.h"
#include "llvm/IR/InstrTypes.h"

#include "ValueLifeInfo.hpp"
#include "logging/logger.hpp"
#include "InstructionScheduler.hpp"

#define  IS_DBG 1

using namespace std;
using namespace hdbe;
using Twine       = llvm::Twine;
using Value       = llvm::Value;
using Instruction = llvm::Instruction;
using Function    = llvm::Function;
using Module      = llvm::Module;

void InstructionScheduler::schedule(BasicBlock * irBasicBlock){};

void InstructionScheduler::schedule() {
 this->schedule(CDI_h->irFunction);
}

void InstructionScheduler::schedule(Function * irFunction) 
{
  auto &M              = *(CDI_h->irModule);
  auto &F              = *irFunction;
  auto &HWD            = CDI_h->HWD;
  auto &stateList      = CDI_h->stateList;
  auto &VIM            = CDI_h->valueInfoMap;
  auto &entryBlock     = *(F.begin());
  
  LOG(INFO, "Scheduling of function " << F.getName());  
  
  // Map instructions to SPACE and TIME        
        
    
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

  LOG(INFO, "Collecting instructions for scheduling");
  for(auto bb_i = F.begin(), bb_end = F.end(); bb_i != bb_end ; ++bb_i)
  {
      LOG_S(IS_DBG) << " Block : " << bb_i->getName() << "\n";
      std::list<Instruction * > instructions;                      
      //Collecting all instructions in the basic block 
      for(auto ins_i = bb_i->begin(), ins_end = bb_i->end(); ins_i != ins_end; ++ ins_i)
      {
        instructions.push_back(&*ins_i);
        
        //Update name
        if (ins_i->getName().empty() && !ins_i->getType()->isVoidTy()) {
          ins_i->setName(Twine('s') + Twine::utohexstr(reinterpret_cast<intptr_t>(&*ins_i)));
        }        
        
        LOG_S(IS_DBG) << "Instruction: " << &*ins_i << "  " << *ins_i << "\n";              
      }
    
  
    //Scheduling 
    uint32_t step = 0;     
    //For each basic block do until all instructions in that is scheduled 
    while( ! instructions.empty()) 
    {
      stateList.push_back(HdlState(&*bb_i, step));
      HdlState & state  = stateList.back(); 
      
      //Iterate the list and see if the instruction can be scheduled  
      //Instruction can be scheduled when all the operands are valid 
      for (auto list_i = instructions.begin(), list_end = instructions.end(); list_i != list_end;)
      {
        Instruction* I = *list_i;
        //Get Operands           
        float dependency_valid = step;
        LOG_S(IS_DBG + 1) << *I << " with opcode " << I->getOpcodeName() << "\n";
          switch(I->getOpcode())
          {
            case llvm::Instruction::PHI: break;
            default:
              
              //Find the latest time of dependency  
              LOG_S(IS_DBG + 2) << "Processing operand\n";
              
              for(const llvm::Use &use : I->operands())
              {
                llvm::Value* val = use.get();
                LOG_S(IS_DBG + 3) << val << " " << *val << "\n";
                
                //Don't check if the operand is constant
                if (llvm::Constant::classof(val)) continue;
                
                if (VIM.count(val) == 0) {
                  dependency_valid = 1.0e6; 
                  LOG_S(IS_DBG + 3) << "Not found \n"; 
                  break;
                }

                float operand_valid = VIM[val].birthTime.time;
               
                if (dependency_valid < operand_valid) dependency_valid = operand_valid;
                              
              }
          }
          
          float latency    = HWD.getLatency(I);

          float valid_time = HWD.getValidTime(I, dependency_valid);

          LOG_S(IS_DBG + 1) << "Timing info: " << dependency_valid << " " << valid_time << "\n";
                  
          
          //Ok to be schedule 
          if (valid_time <= (step + 1.0)) {
            
            if (I->isTerminator())
              state.termInstruction = I;
            else 
              state.instructionList.push_back(I);  
            
            auto ret = CDI_h->addValueInfo(I);
            
            ret.first->second.setBirthTime(&state, valid_time);
            
            LOG_S(IS_DBG) << "ok to be scheduled, valid time " << ret.first->second.birthTime.time << "\n";            
            
            //Update usage time 
            //The instruction is scheduled, we update the operands useage 
            for(const llvm::Use &use : I->operands())
              {
                VIM[(llvm::Value*)use.get()].addUseTime(&state, step);
              }
            
            //Finally erase the item 
            list_i = instructions.erase(list_i);

          } else {

            ++ list_i;

          }
        }   
        step ++ ; 
        //For debugging
        if (step > 10) return;
    }
  }
  LOG(INFO, "... done ");
}

