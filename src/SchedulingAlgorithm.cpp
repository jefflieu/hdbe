#include "Scheduler.hpp"
#include "types.hpp"
#include "loguru/loguru.hpp"
#include "ControlStep.hpp"
#include "llvm/Analysis/DDG.h"
#include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/IR/InstrTypes.h"

using namespace std;
using namespace hdbe;


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
  
  LOG_F(INFO, "SimpleScheduler Visit");  
  
  //Walk the dependence graph and map instruction to "time"        
  std::map<llvm::Value*, float> valueBirthTime;
  
  
  try {       
    
    LOG_IF_S(FATAL, F == NULL) << "Function not found";
    LOG_S(INFO) << "Found Function: " << g_getStdStringName(F);    
    LOG_S(INFO) << "Arguments :";      
    
    //Assign birth time for Global Variable 
    for(auto global_var_i = module->global_begin(), last = module->global_end(); global_var_i != last; ++global_var_i)
    {
      valueBirthTime[D_GET_ITEM_PTR(global_var_i)] = 0;
    }

    //Assign birth_time for Arguments:   
    for(auto arg_i = F->arg_begin(), last = F->arg_end(); arg_i != last; arg_i ++ )
    {
      LOG_S(6) << g_getStdStringName(arg_i);
      valueBirthTime[D_GET_ITEM_PTR(arg_i)] = 0;
    }
  
    
    for(auto bbi = F->begin(), bbe = F->end(); bbi != bbe ; ++bbi)
    {
      LOG_S(6) << " Basic block: " << g_getStdStringName(*bbi);
      std::list<Const_Instruction_h> instructions;                
      
      //Collecting all instructions in the basic block 
      for(auto ins_i = bbi->begin(), instruction_end = bbi->end(); ins_i != instruction_end; ++ ins_i)
      {
        auto ins_h = D_GET_ITEM_PTR(ins_i);
        instructions.push_back(ins_h);
        LOG_S(6) << "Instruction: " << g_getStdStringName(ins_h) <<" ID: " << ins_h;              
      }
     
      
      //Scheduling 
      uint32_t step = 0;
      //For each basic block do until all instructions in that is scheduled 
      while( ! instructions.empty()) {

        const std::string step_name = g_getStdStringName(*bbi) + "." + std::to_string(step);
        ControlStep *cs  = new ControlStep(step_name);
        LOG_S(6) << step_name;
        
        //Iterate the list and see if the instruction can be scheduled  
        //Instruction can be scheduled when all the operands are valid 
        for(auto I = instructions.begin(); I!=instructions.end();)
        {
          //Get Operands           
          bool good = true;
          switch((*I)->getOpcode())
          {
            case llvm::Instruction::PHI: break;
            default: 
              for(auto op_i = (*I)->op_begin(), last = (*I)->op_end(); op_i != last; op_i++)
              {
                if (((*op_i)->getType())->isLabelTy()) break;
                if (((*op_i)->getValueID()) > llvm::Value::ValueTy::ConstantFirstVal && ((*op_i)->getValueID() < llvm::Value::ValueTy::ConstantLastVal)) break;
                good = (valueBirthTime.count(*op_i) > 0);
                if (good) {
                  if (step < valueBirthTime[*op_i]) {good = false; break;}                            
                } else {
                llvm::outs() << (*op_i)->getName() << "Type ID: " << ((*op_i)->getType())->getTypeID() << " Value ID: " << (*op_i)->getValueID() << " not found\n";  
                (*op_i)->dump();
                break;
                }
                //LOG_S(1) << g_getStdStringName((*op_i));              
              }
          }
          auto handle = *I;          
          ++I;
          if (good) {
            cs->addInstruction(handle);  
            instructions.remove(handle);
            valueBirthTime[(llvm::Value*)(handle)] = step + hwd->getLatency(handle);
            LOG_S(6) << "Instruction " <<g_getStdStringName(handle) << " (" << handle << ") " << " is scheduled";
          }
        }
        scheduler->addControlStep(cs);
        step ++ ; 
        if (step > 10) return 0x1;
      }

    }
  }
  catch (...) {    
  }
  return 0;
}

