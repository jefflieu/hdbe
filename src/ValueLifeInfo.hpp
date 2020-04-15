#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Value.h"

#include "HardwareDescription.hpp"
#include "CodeGenerator.hpp"
#include "ControlStep.hpp"
#include "BaseClass.hpp"
#include "types.hpp"
#include "loguru/loguru.hpp"




using Value = llvm::Value;
using BasicBlock = llvm::BasicBlock;

namespace hdbe {
class ValueLifeInfo : public BaseClass {
  struct TimePoint {    
    BasicBlock *bb;
    float       step;
  };

  public: 
    Value* irValue;
    std::vector<TimePoint> useTimeList;
    TimePoint birth;

  public: 
    ValueLifeInfo(): irValue(nullptr) {};
    ValueLifeInfo(Value* _val): irValue(_val) {};
    ~ValueLifeInfo() {};
    void setBirthTime(BasicBlock *bb, float step){birth.bb = bb; birth.step = step; LOG_S(6) << irValue << " " << bb << " " << step;};
    BasicBlock *getBirthTimeBB(){return birth.bb;}
    float       getBirthTimeStep(){return birth.step;}
    TimePoint&  getBirthTime() {return birth;}
    void addUseTime(BasicBlock *bb, float step ){TimePoint tmp = {.bb = bb, .step = step};  useTimeList.push_back(tmp);}
    std::vector<TimePoint>& getUseTimeList() {return useTimeList;}
    void dump() {LOG_S(6) << irValue << " " << birth.bb << " " << birth.step;};
    int getRegisterStage() {
      return floor(useTimeList.back().step) - floor(birth.step);
    }
};

}