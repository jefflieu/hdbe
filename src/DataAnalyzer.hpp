#pragma once 
#include <string> 
#include <list> 
#include <iostream> 

#include "BaseClass.hpp"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "OSPrint.hpp"
#include "HdlObject.hpp"

namespace hdbe {



class DataAnalyzer : public BaseClass {
  //
  public: 
    std::vector<HdlPort> m_portList;
  
  private: 
    llvm::Module *m_irModule;
  public: 
    DataAnalyzer (llvm::Module *module) :  m_irModule(module){};
    ~DataAnalyzer() {}

    void analyze(std::string funcname);
    HdlProperty analyzePointer(llvm::Value* valuePointerTy);
};

}