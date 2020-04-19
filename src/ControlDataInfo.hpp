#pragma once 

#include <typeinfo> 

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "HdlObject.hpp"

namespace hdbe {

using Module = llvm::Module;
using Function = llvm::Function;

class ControlDataInfo {
  
  friend class DataAnalyzer;
  
  protected: 
    Module   *irModule   = nullptr; 
    Function *irFunction = nullptr;
    
  
    std::vector<HdlPort> portList;
    std::vector<HdlVariable> variableList;
  

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

