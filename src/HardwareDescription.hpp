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
  
  private: 
    ControlDataInfo* CDI_h;
  
  public: 
    HardwareDescription () {};
    ~HardwareDescription() {};

    void setParent(ControlDataInfo* cdi) {CDI_h = cdi;}
    float getLatency(llvm::Instruction* instruction);
    float getValidTime(llvm::Instruction* instruction, float latest_dependency);

};

}
