#include "Scheduler.hpp"
#include "types.hpp"
#include "ControlStep.hpp"
#include "ValueLifeInfo.hpp"
#include "logging/logger.hpp"

#include "llvm/Analysis/DDG.h"
#include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/InstrTypes.h"

using namespace std;
using namespace hdbe;
using Twine = llvm::Twine;
using Value = llvm::Value;
using Instruction = llvm::Instruction;


uint32_t  SimpleScheduler::schedule(SchedulingAlgorithm& algo, std::string funcName) {
  m_function = m_module->getFunction(funcName);
  return algo.visit(this);
}

uint32_t  BasicBlockScheduler::schedule(SchedulingAlgorithm& algo, BasicBlock_h bb) {  
  return algo.visit(this, bb);
}

uint32_t SchedulingAlgorithm::visit(BasicBlockScheduler* scheduler, BasicBlock_h bb) {

  return 0;
}
    
uint32_t SchedulingAlgorithm::visit(SimpleScheduler* scheduler) 
{
  auto module = scheduler->m_module;
  auto F      = scheduler->m_function;
  auto hwd    = &(scheduler->HWD);
  auto cslist = &(scheduler->m_ctrlSteps);
  auto &valueInfoMap   = scheduler->valueInfoMap;
  auto firstBB = &*(F->begin());
  
  LOG_F(INFO, "SimpleScheduler Visit");  
  
  //Walk the dependence graph and map instruction to "time"        
  try {       
    LOG_IF_S(FATAL, F == NULL) << "Function not found";
    LOG_S(INFO) << "Found Function: " << g_getStdStringName(F);    
    LOG_S(INFO) << "Arguments :";      
    
    //Assign birth time for Global Variable 
    for(auto global_var_i = module->global_begin(), last = module->global_end(); global_var_i != last; ++global_var_i)
    {
      LOG_S(6) << &*global_var_i << " name " << g_getStdStringName(*global_var_i);
      auto ret = valueInfoMap.insert(std::pair<llvm::Value*, ValueLifeInfo>(&*global_var_i, ValueLifeInfo(&*global_var_i)));
      ret.first->second.setBirthTime(firstBB, 0.0);
    }

    //Assign valid_time for Arguments:   
    for(auto arg_i = F->arg_begin(), last = F->arg_end(); arg_i != last; arg_i ++ )
    {
      LOG_S(6) << &*arg_i << " name " << g_getStdStringName(*arg_i);
      auto ret = valueInfoMap.insert(std::pair<llvm::Value*, ValueLifeInfo>(&*(arg_i), ValueLifeInfo(&*(arg_i))));
      ret.first->second.setBirthTime(firstBB, 0.0);      
    }

 
    for(auto BBi = F->begin(), bbe = F->end(); BBi != bbe ; ++BBi)
    {
      LOG_S(6) << " Basic block: " << g_getStdStringName(*BBi);
      std::list<Const_Instruction_h> instructions;                      
      //Collecting all instructions in the basic block 
      for(auto ins_i = BBi->begin(), instruction_end = BBi->end(); ins_i != instruction_end; ++ ins_i)
      {
        auto ins_h = D_GET_ITEM_PTR(ins_i);
        instructions.push_back(ins_h);
        if (ins_h->getName().empty() && !ins_h->getType()->isVoidTy()) {ins_h->setName(Twine('s') + Twine::utohexstr(reinterpret_cast<intptr_t>(ins_h)));}        
        LOG_S(6) << "Instruction: " << g_getStdStringName(ins_h) <<" ID: " << ins_h;              
      }
     
      
      //Scheduling 
      uint32_t step = 0;     
      //For each basic block do until all instructions in that is scheduled 
      while( ! instructions.empty()) {

        cslist->push_back(ControlStep(&*BBi, step));
        ControlStep& cs  = cslist->back();
        
        //Iterate the list and see if the instruction can be scheduled  
        //Instruction can be scheduled when all the operands are valid 
        for(auto I = instructions.begin(); I!=instructions.end();)
        {
          //Get Operands           
          float dep_birthtime = step;
          LOG_S(6) << "Instruction " << (*I) << " " << g_getStdStringName(*I) << " " << (*I)->getOpcodeName();
          switch((*I)->getOpcode())
          {
            case llvm::Instruction::PHI: break;
            default:
              //Find the latest time of dependency  
              LOG_S(6) << "Processing operand";
              for(const llvm::Use &use : (*I)->operands())
              {
                llvm::Value* val = use.get();
                LOG_S(6) << val;
                val->dump();
                if ((val->getType())->isLabelTy()) continue;
                if ((val->getValueID()) > llvm::Value::ValueTy::ConstantFirstVal && (val->getValueID() < llvm::Value::ValueTy::ConstantLastVal)) continue;
                if (valueInfoMap.count(val) == 0) {dep_birthtime = 1.0e6; LOG_S(6) << "Not found"; break;}
                float op_i_birth = valueInfoMap[val].getBirthTimeStep();
                LOG_S(6) << val << " " << val->getName().str() << " " <<op_i_birth;
                if (dep_birthtime < op_i_birth) dep_birthtime = op_i_birth;
                
                //llvm::outs() << val->getName() << "Type ID: " << (val->getType())->getTypeID() << " Value ID: " << val->getValueID() << " not found\n";  
                //val->dump();                
              }
          }
          float latency = hwd->getLatency(*I);          
          float valid_time = hwd->getValidTime(*I, dep_birthtime) ;          
          LOG_S(6) << "Timing info: " << dep_birthtime << " " << valid_time;
          auto instruction = *I;          
          ++I;
          //Ok to be schedule 
          if (valid_time <= (step + 1.0)) {
            cs.setBranch(instruction->isTerminator()); //Should be the last one to be in the list
            cs.setReturn(instruction->getOpcode() == llvm::Instruction::Ret); //Should be the last one to be in the list
            cs.addInstruction(instruction);  
            instructions.remove(instruction);
            auto ret = valueInfoMap.insert(std::pair<llvm::Value*, ValueLifeInfo>((llvm::Value*)instruction, ValueLifeInfo((llvm::Value*)instruction)));
            ret.first->second.setBirthTime(&*BBi, valid_time);
            LOG_S(6) << "ok to be scheduled, valid time " << ret.first->second.getBirthTimeStep();            
            //Update usage time 
            //The instruction is scheduled, we update the operands useage 
            for(const llvm::Use &use : instruction->operands())
              {
                valueInfoMap[(llvm::Value*)use.get()].addUseTime(&*BBi, step);
              }
          }
        }  
        LOG_S(1) << cs;
        step ++ ; 
        if (step > 10) return 0x1;
      }

    }
  }
  catch (...) {    
  }

  return 0;
}

