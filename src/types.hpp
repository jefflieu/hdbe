
#pragma once 
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"


namespace hdbe {

class ControlStep;

typedef llvm::Instruction* Instruction_h;
typedef llvm::Module* Module_h ;
typedef ControlStep* ControlStep_h;
typedef llvm::Function* Function_h;
typedef llvm::BasicBlock* BasicBlock_h;

typedef const llvm::Instruction* Const_Instruction_h;
typedef const llvm::Module* Const_Module_h ;
typedef const ControlStep* Const_ControlStep_h;
typedef const llvm::Function* Const_Function_h;
typedef const llvm::BasicBlock* Const_BasicBlock_h;


/* 
  These helper helps to convert to STD string from StringRef, 
  Work both cases of argument being pointer or a reference
*/
template<typename T>
std::string g_getStdStringName(T* irObject) {return (irObject->getName()).str();}

template<typename T>
std::string g_getStdStringName(T& irObject) {return (irObject.getName()).str();}

/* 
  Lots of iterator in the LLVM when performing * will point to a reference 
  Use D_GET_PTR to get the Pointer.
*/
#define D_GET_ITEM_PTR(iter) (&(*iter))


#define D_IS_CONST_OPERAND(oprnd) (oprnd->getValueID() > llvm::Value::ValueTy::ConstantFirstVal) && (oprnd->getValueID() < llvm::Value::ValueTy::ConstantLastVal)

}

