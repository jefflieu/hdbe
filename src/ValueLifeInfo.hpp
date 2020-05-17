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
class ValueLifeInfo {
  struct TimePoint {    
    HdlState * state = nullptr;
    float      time = 0;
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
    
    void setBirthTime(HdlState *state, float time);
    
    void addUseTime(HdlState *state, float time);
    
    UseTimeListType& getUseTimeList() {return useTimeList;}
    
    //Assuming all are in same block
    int getLiveTime();

    String repr() {
      String s;
      s += "ValueLifeInfo of " + getBriefInfo(irValue) + " : "; 
      s += "" + std::to_string(birthTime.time) +  ",";
      if (useTimeList.empty())
        s += "no user\n";
      else 
        s += std::to_string(useTimeList.back().time) + "\n";
      return s;
    }
};

}