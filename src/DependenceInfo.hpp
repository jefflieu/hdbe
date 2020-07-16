/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/StringExtras.h"

#include "logging/logger.hpp"


#include "IRUtil.hpp"
#include "HdlState.hpp"

using Value = llvm::Value;
using BasicBlock = llvm::BasicBlock;
using String = std::string;

namespace hdbe {

/*
  Instruction A depends on Instruction B for the purpose of scheduling
  There are 2 types of dependence that we're concerned about 
  Valid    : B is required to be Valid for A to be scheduled
  Schedule : B is required to be scheduled for A to be scheduled. For example: 
             s1 = load(XX) (A instruction)
             store(s2, YY) (B instruction), XX and YY is unknown at compile time and can be the same address location
             Due to the latency of the instruction, A and B can be executed in the same cycle 
*/
enum class DependenceType {
  Valid        = 0,
  Schedule     = 1,
  NoDependency = 2
}; 

class DependenceInfo {
  
  using DependenceMap  = std::map<Value*, DependenceType>;
  const String DepType[3] = {"Valid", "Schedule", "None"};
  private: 
    Value              *irValue;
    
    DependenceMap DIM;    

  public: 
    DependenceInfo() {};
    DependenceInfo(Value* _val): irValue(_val) {};
    ~DependenceInfo() {};
    void addDependence(Value* val, DependenceType type) {DIM[val] = type;}
    DependenceMap & getDependenceMap() {return DIM;}
    String repr() {
      String s;
      s += "::" + getBriefInfo(irValue) + " depends on: \n"; 
      for(auto &item : DIM)
        {
          s+= getBriefInfo(item.first)  + " - " + DepType[(int)item.second] + "\n";
        }
      return s;
    }   
};

}