#include "Scheduler.hpp"
#include "types.hpp"

using namespace hdlbe;
using namespace llvm;


uint32_t  AsapScheduler::schedule(SchedulingAlgorithm& algo, std::string funcName) {
  return algo.visit(this, funcName);
}
    
uint32_t SchedulingAlgorithm::visit(AsapScheduler* scheduler, std::string funcName) 
{
  //Iterate a round m_irModule 
  std::cout << "AsapScheduler Visit" << '\n';
  // Go over all named mdnodes in the module
  for (Module::const_iterator I = scheduler->m_irModule->begin(), E = scheduler->m_irModule->end(); I != E; ++I) {
    
    // These dumps only work with LLVM built with a special cmake flag enabling
    // dumps.
    //I->dump();
    outs() << "Found Function: " << I->getName() << '\n';
    if (I->getName() != funcName) continue;
    outs() << "Arguments :" << '\n';
    for(Function::const_arg_iterator ai = I->arg_begin(), ae = I->arg_end(); ai != ae; ++ai)
    {
      ai->dump();
    }

    for(Function::const_iterator bbi = I->begin(), bbe = I->end(); bbi != bbe ; ++bbi)
    {
      outs() << " Basic block " <<bbi->getName() << '\n';
      for(BasicBlock::const_iterator ins_i = bbi->begin(), instruction_end = bbi->end(); ins_i != instruction_end; ++ ins_i)
      {
        ins_i->dump();
      }
    }
  }
}

