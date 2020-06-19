/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 
#include <string> 

#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

namespace hdbe {

using ValuePtrVector = llvm::SmallVector<llvm::Value*, 16>; 
using BasicBlock     = llvm::BasicBlock; 
using Instruction    = llvm::Instruction; 
using Value          = llvm::Value; 
using Twine          = llvm::Twine; 
using StringRef      = llvm::StringRef; 
using String         = std::string;

bool isMemoryInstruction(Instruction* I);
bool isMemoryInstruction(Instruction& I);
bool isPHIInstruction(Instruction* I);
bool isCtrlFlowInstruction(Instruction* I);
int computeIndex(Instruction* I, llvm::Value* basePtr);
bool isUselessInstruction(Instruction* I);
ValuePtrVector getInstructionInputs(Instruction* I, bool exclude_backedge = true);
ValuePtrVector getInstructionOutputs(Instruction* I);
String getBriefInfo(Value* val);
bool isBackEdge(BasicBlock *src, BasicBlock *dst);
}
