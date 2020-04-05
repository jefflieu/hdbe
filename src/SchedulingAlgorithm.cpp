#include "Scheduler.hpp"
#include "types.hpp"
#include "loguru/loguru.hpp"
#include "ControlStep.hpp"
#include "llvm/Analysis/DDG.h"
#include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/IR/InstrTypes.h"

using namespace std;
using namespace hdlbe;


uint32_t  SimpleScheduler::schedule(SchedulingAlgorithm& algo, std::string funcName) {
  constructDDG(funcName);
  return algo.visit(this, funcName);
}
    
uint32_t SchedulingAlgorithm::visit(SimpleScheduler* scheduler, std::string funcName) 
{
  LOG_F(INFO, "SimpleScheduler Visit");  
  
  //Walk the dependence graph and map instruction to "time"        
  std::map<llvm::Value*, float> valueBirthTime;
  
  
    
  try {       
    
    Function_h F = scheduler->m_irModule->getFunction(funcName);
    LOG_IF_S(FATAL, F == NULL) << "Function not found";
    LOG_S(INFO) << "Found Function: " << GetName(F);    
    LOG_S(INFO) << "Arguments :";      
    
    //Assign birth time for Global Variable 
    for(auto global_var_i = scheduler->m_irModule->global_begin(), last = scheduler->m_irModule->global_end(); global_var_i != last; ++global_var_i)
    {
      valueBirthTime[D_GET_PTR(global_var_i)] = 0;
    }

    //Assign birth_time for Arguments:   
    for(auto arg_i = F->arg_begin(), last = F->arg_end(); arg_i != last; arg_i ++ )
    {
      LOG_S(6) << GetName(arg_i);
      valueBirthTime[D_GET_PTR(arg_i)] = 0;
    }
  
    
    for(auto bbi = F->begin(), bbe = F->end(); bbi != bbe ; ++bbi)
    {
      LOG_S(6) << " Basic block: " << GetName(*bbi);
      std::list<Const_Instruction_h> instructions;                
      
      //Collecting all instructions in the basic block 
      for(auto ins_i = bbi->begin(), instruction_end = bbi->end(); ins_i != instruction_end; ++ ins_i)
      {
        auto ins_h = D_GET_PTR(ins_i);
        instructions.push_back(ins_h);
        LOG_S(6) << "Instruction: " << GetName(ins_h) <<" ID: " << ins_h;              
      }
      
      //Scheduling 
      uint32_t step = 0;
      //For each basic block do until all instructions in that is scheduled 
      while( ! instructions.empty()) {

        const std::string step_name = GetName(*bbi) + "." + std::to_string(step);
        ControlStep *cs  = new ControlStep(step_name);
        LOG_S(6) << step_name;
        for(auto I = instructions.begin(); I!=instructions.end();)
        {
          //Get Operands           
          bool good = true;
          for(auto op_i = (*I)->op_begin(), last = (*I)->op_end(); op_i != last; op_i++)
          {
            good = (valueBirthTime.count(*op_i) > 0);
            if (good) {
              if (step < valueBirthTime[*op_i]) {good = false; break;}                            
            } else break;
          }
          
          auto I_Last = I;
          ++I;
          if (good) {
            cs->addInstruction(* I_Last);
            instructions.remove(*I_Last);
            valueBirthTime[(llvm::Value*)(*I_Last)] = step + scheduler->HWD.getLatency(*I_Last);
            LOG_S(6) << "Instruction " <<GetName(*I_Last) << " (" << *I_Last << ") " << " is scheduled";
          }
        }
        scheduler->addControlStep(cs);
        step ++ ;        
      }

    }
  }
  catch (...) {    
  }
}

