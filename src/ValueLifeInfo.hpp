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
    float time;
  };
  using UseTimeListType = std::vector<TimePoint>;

  public: 
    Value              *irValue;
    UseTimeListType    useTimeList;
    TimePoint          schedule;
    TimePoint          valid;

  public: 
    ValueLifeInfo(): irValue(nullptr), schedule({-1.0}), valid({-1}) {};
    ValueLifeInfo(Value* _val): irValue(_val), schedule({-1.0}), valid({-1}) {};
    ~ValueLifeInfo() {};
    
    void setBirthTime(float schedule, float valid);
    
    void addUseTime(float time);

    float getScheduledTime() {return schedule.time;}
    float getValidTime() {return valid.time;}
    float getLatestUseTime() {return useTimeList.empty()?-1:useTimeList.back().time;}
    
    UseTimeListType& getUseTimeList() {return useTimeList;}
    
    String repr() {
      String s;
      s += "ValueLifeInfo of " + getBriefInfo(irValue) + " : "; 
      s += "" + std::to_string(getScheduledTime()) +  ",";
      s += "" + std::to_string(getValidTime()) +  ",";
      if (useTimeList.empty())
        s += "no user\n";
      else 
        s += std::to_string(getLatestUseTime()) + "\n";
      return s;
    }
};

}