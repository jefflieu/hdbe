
#pragma once 
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"

namespace hdlbe {

class ControlStep;

typedef llvm::Instruction* Instruction_h;
typedef llvm::Module* Module_h ;
typedef ControlStep* ControlStep_h;

#define GET_HANDLE_FROM_UNIQUE_PTR( u_ptr ) (&(*u_ptr))

}

