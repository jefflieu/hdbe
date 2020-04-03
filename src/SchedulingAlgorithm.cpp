#include "Scheduler.hpp"
#include "types.hpp"
#include "loguru/loguru.hpp"

using namespace hdlbe;
using namespace llvm;


uint32_t  AsapScheduler::schedule(SchedulingAlgorithm& algo, std::string funcName) {
  return algo.visit(this, funcName);
}
    
uint32_t SchedulingAlgorithm::visit(AsapScheduler* scheduler, std::string funcName) 
{
  //Iterate a round m_irModule 
  LOG_F(INFO, "AsapScheduler Visit");
  
  try {       
    
    Function_h F = scheduler->m_irModule->getFunction(funcName);
    LOG_IF_S(FATAL, F == NULL) << "Function not found";
    LOG_S(INFO) << "Found Function: " << GetName(F);    
    LOG_S(INFO) << "Arguments :";      
    for(Function::const_arg_iterator ai = F->arg_begin(), ae = F->arg_end(); ai != ae; ++ai)
    {
      ai->dump();
    }

    for(Function::const_iterator bbi = F->begin(), bbe = F->end(); bbi != bbe ; ++bbi)
    {
      LOG_S(1) << " Basic block: " << GetName(*bbi);//(bbi->getName()).str();
      for(BasicBlock::const_iterator ins_i = bbi->begin(), instruction_end = bbi->end(); ins_i != instruction_end; ++ ins_i)
      {
        ins_i->dump();
      }
    }
  }
  catch (...) {    
  }
}

