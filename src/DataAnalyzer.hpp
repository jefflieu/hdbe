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
  template<typename T> 
  bool isIn(std::vector<T>& v, llvm::Value* val){
    for(auto I = v.begin(), E = v.end(); I!=E; ++I)
      {
        if ((*I).m_irValue == val) return true;
      }
    return false;
  }
  public: 
    std::vector<HdlPort> m_portList;
    std::vector<HdlVariable> m_variableList;
  
  private: 
    llvm::Module *m_irModule;
  public: 
    DataAnalyzer (llvm::Module *module) :  m_irModule(module){};
    ~DataAnalyzer() {}

    void analyze(std::string funcname);
    HdlProperty analyzePointer(llvm::Value* valuePointerTy);
    HdlProperty analyzeValue(llvm::Value* value);
};

}