/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include <typeinfo> 

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constant.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Hashing.h"

#include "logging/logger.hpp"

namespace hdbe {


class IRPreprocessor {

  using Module = llvm::Module;
  using Function = llvm::Function;
  using BasicBlock = llvm::BasicBlock; 
  using hash_code = llvm::hash_code; 
  using Twine = llvm::Twine; 
  using Instruction = llvm::Instruction;
  using StringRef = llvm::StringRef;
  using User = llvm::User;
  using ConstantExpr = llvm::ConstantExpr;
  using String = std::string;
  using InstPointerVector = llvm::SmallVector<Instruction*, 16>;

  private: 
    Module   *irModule   = nullptr; 
    Function *irFunction = nullptr;

  public :
    IRPreprocessor () {};     
    IRPreprocessor (Module *_module, const char* functionName) : irModule(_module) 
      { 
        this->irFunction = irModule->getFunction(functionName);
        if (!irFunction) LOG(ERROR, "Function named " << functionName << " .. not found");
        else LOG(INFO, "Module loaded");  
      }
    ~IRPreprocessor () {}

    void run() { removePointerPHI(); processConstantExpr(); transformNames();}// balanceCFG();}
    void transformNames();
    void balanceCFG();
    void removePointerPHI();
    void getRelatedValues(Instruction* I, BasicBlock* bb, InstPointerVector & ipv);
    bool processConstantExpr(Instruction* I);
    void processConstantExpr();
};

}
