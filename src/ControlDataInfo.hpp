#pragma once 

#include <typeinfo> 

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "HdlObject.hpp"

namespace hdbe {

using Module      = llvm::Module;
using Function    = llvm::Function;
using Instruction = llvm::Instruction;

class ControlDataInfo {
  
  friend class DataAnalyzer;
  friend class VerilogGenerator;
  
  protected: 
    Module   *irModule   = nullptr; 
    Function *irFunction = nullptr;
    
  
    std::list<HdlPort     > portList;
    std::list<HdlVariable > variableList;
    std::list<HdlVariable > memOpsList;
  

  public :
    ControlDataInfo () {};     
    ControlDataInfo (Module *_module, const char* functionName) : irModule(_module) 
      { 
        this->irFunction = irModule->getFunction(functionName);
        if (!irFunction) 
          LOG(ERROR, "Function named " << functionName << " .. not found");
        else 
          LOG(INFO, "Module loaded");  
      }
    ~ControlDataInfo () {}


};


}

