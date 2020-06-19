/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 
#include <string> 
#include <list> 
#include <map> 
#include <iostream> 

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"

#include "HdlObject.hpp"
#include "ControlDataInfo.hpp"

namespace hdbe {


class DataAnalyzer {

  using Function    = llvm::Function;
  using GlobalValue = llvm::GlobalValue;
  using DataLayout  = llvm::DataLayout;
  using Module      = llvm::Module;
  using Argument    = llvm::Argument;
  using ConstantInt = llvm::ConstantInt;
  using User        = llvm::User;
  using Use         = llvm::Use;


  template<typename T> 
  bool isIn(std::list<T>& v, llvm::Value* val){
    for(auto I = v.begin(), E = v.end(); I!=E; ++I)
      {
        if ((*I).getIrValue()  == val) return true;
      }
    return false;
  }

  template<typename T> 
  T& find(std::list<T>& v, llvm::Value* val){
    for(auto I = v.begin(), E = v.end(); I!=E; ++I)
      {
        if ((*I).getIrValue()  == val) return *I;
      }
    return *v.end();
  }
 
  private: 
    ControlDataInfo *CDI_h;

    bool isBackValue(Value* v);

  public: 
    DataAnalyzer (ControlDataInfo *cdi_h) :  CDI_h(cdi_h){};
    ~DataAnalyzer() {}

    void analyze(Module * irModule, Function * irFunction);
    void analyze() {
      analyzeLoops(CDI_h->irModule, CDI_h->irFunction);
      analyzeBasicBlocks(CDI_h->irModule, CDI_h->irFunction);
      analyze(CDI_h->irModule, CDI_h->irFunction);
      }
    HdlProperty analyzePointer(llvm::Value* valuePointerTy);
    HdlProperty analyzeValue(llvm::Value* value);
    static Value* analyzeMemoryOp(Instruction * memOp, int* index);
    void analyzeBasicBlocks(Module* irModule, Function* irFunction);
    void analyzeLoops(Module* irModule, Function* irFunction);

};

}