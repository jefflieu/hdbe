
#pragma once 
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"

namespace hdlbe {

class ControlStep;

typedef llvm::Instruction* Instruction_h;
typedef llvm::Module* Module_h ;
typedef ControlStep* ControlStep_h;
typedef llvm::Function* Function_h;

template<typename T>
std::string GetName(T* irObject) {return (irObject->getName()).str();}

template<typename T>
std::string GetName(T& irObject) {return (irObject.getName()).str();}
}

