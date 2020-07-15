/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 
#include <string> 
#include <list> 
#include <iostream> 

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"

#include "HdlObject.hpp"
#include "ControlDataInfo.hpp"

namespace hdbe {


class InstructionScheduler {

  using Module     = llvm::Module;
  using Function   = llvm::Function;
  using BasicBlock = llvm::BasicBlock;
  using Instruction = llvm::Instruction;
  using Twine       = llvm::Twine;
  using Value       = llvm::Value;
  using User        = llvm::User;
  using LoopInfo    = llvm::LoopInfo;

  private: 
    ControlDataInfo *CDI_h;
    bool isBackValue(Value* v);
    bool isMemoryInstSchedulable(Instruction * brInst, float current_step);
    bool isBranchSchedulable(Instruction * brInst, float current_step);
    bool isBasicBlockSchedulable(BasicBlock * bb);
    float getDependencyValidTime(Instruction* instr);
    void updateBackValueUseTime(Instruction* instr);


  public: 
    InstructionScheduler (ControlDataInfo *cdi_h) :  CDI_h(cdi_h){};
    ~InstructionScheduler() {}

    void schedule(Function * irFunction);
    void schedule(BasicBlock * irBasicBlock);
    void schedule();
    void dumpInstructions(std::list<Instruction*> &instList);
};

}