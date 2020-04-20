#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Value.h"

#include "logging/logger.hpp"




using Value = llvm::Value;
using BasicBlock = llvm::BasicBlock;


namespace hdbe {
class ValueLifeInfo {
  struct TimePoint {    
    BasicBlock *block;
    float       step;
  };

  using UseTimeListType = std::vector<TimePoint>;

  public: 
    Value              *irValue;
    UseTimeListType    useTimeList;
    TimePoint          birthTime;

  public: 
    ValueLifeInfo(): irValue(nullptr) {};
    ValueLifeInfo(Value* _val): irValue(_val) {};
    ~ValueLifeInfo() {};
    
    void setBirthTime(BasicBlock *bb, float step);
    
    void addUseTime(BasicBlock *bb, float step);
    
    UseTimeListType& getUseTimeList() {return useTimeList;}
    
    //Assuming all are in same block
    int getLiveTime();
};

}