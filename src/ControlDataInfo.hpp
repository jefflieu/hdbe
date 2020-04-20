#pragma once 

#include <typeinfo> 

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "HdlObject.hpp"
#include "HdlState.hpp"
#include "ValueLifeInfo.hpp"


namespace hdbe {

using Module      = llvm::Module;
using Function    = llvm::Function;
using Instruction = llvm::Instruction;
using Value       = llvm::Value;

class ControlDataInfo {
  
  friend class DataAnalyzer;
  friend class VerilogGenerator;
  friend class InstructionScheduler;

  protected: 
    Module   *irModule   = nullptr; 
    Function *irFunction = nullptr;
    
  
    std::list<HdlPort     > portList;
    std::list<HdlVariable > variableList;
    std::list<HdlVariable > memOpsList;
    std::list<HdlState    > stateList;    
    std::map<Value*, ValueLifeInfo> valueInfoMap;

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

    inline auto addValueInfo (Value* val) {
      return valueInfoMap.insert(std::pair<Value*, ValueLifeInfo>(val, ValueLifeInfo(val)));
    }

};


}

