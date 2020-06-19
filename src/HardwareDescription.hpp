/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 
#include <string> 
#include <list> 


#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

#include "IRUtil.hpp"

namespace hdbe {
class ControlDataInfo;

class HardwareDescription {
  public:
    struct ExecutionInfo
    {
      float latency = -1.0;
      int pipeline = 0;
      float valid = -1.0;
    };
  
  private: 
    ControlDataInfo* CDI_h;
  
  public: 
    HardwareDescription () {};
    ~HardwareDescription() {};

    void setParent(ControlDataInfo* cdi) {CDI_h = cdi;}
    ExecutionInfo requestToSchedule(llvm::Instruction* instruction, float latest_dependency);
};

}
