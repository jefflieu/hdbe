#pragma once 
#include <string> 
#include <list> 
#include <iostream> 

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"

#include "HdlObject.hpp"
#include "ControlDataInfo.hpp"

namespace hdbe {

using Module     = llvm::Module;
using Function   = llvm::Function;
using BasicBlock = llvm::BasicBlock;

class InstructionScheduler {

  private: 
    ControlDataInfo *CDI_h;
  public: 
    InstructionScheduler (ControlDataInfo *cdi_h) :  CDI_h(cdi_h){};
    ~InstructionScheduler() {}

    void schedule(Function * irFunction);
    void schedule(BasicBlock * irBasicBlock);
    void schedule();
};

}