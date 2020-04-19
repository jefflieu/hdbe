#pragma once 
#include <string> 
#include <list> 
#include <iostream> 

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"

#include "OSPrint.hpp"
#include "HdlObject.hpp"
#include "ControlDataInfo.hpp"

namespace hdbe {

using Module   = llvm::Module;
using Function = llvm::Function;

class DataAnalyzer {

  template<typename T> 
  bool isIn(std::vector<T>& v, llvm::Value* val){
    for(auto I = v.begin(), E = v.end(); I!=E; ++I)
      {
        if ((*I).getIrValue()  == val) return true;
      }
    return false;
  }
 
  private: 
    ControlDataInfo *CDI_h;
  public: 
    DataAnalyzer (ControlDataInfo *cdi_h) :  CDI_h(cdi_h){};
    ~DataAnalyzer() {}

    void analyze(Module * irModule, Function * irFunction);
    void analyze() {analyze(CDI_h->irModule, CDI_h->irFunction);}
    HdlProperty analyzePointer(llvm::Value* valuePointerTy);
    HdlProperty analyzeValue(llvm::Value* value);
};

}