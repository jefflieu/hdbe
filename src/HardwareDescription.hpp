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
  using ResourceMap = std::map<unsigned, unsigned>;
  public:
    struct ExecutionInfo
    {
      float latency = -1.0;
      int pipeline = 0;
      float valid = -1.0;
      bool hw_available = true;
    };
  
  private: 
    ControlDataInfo* CDI_h;
    ResourceMap   RM;
  
  public: 
    HardwareDescription () {};
    ~HardwareDescription() {};

    void setParent(ControlDataInfo* cdi) {CDI_h = cdi;}
    ExecutionInfo requestToSchedule(llvm::Instruction* instruction, float latest_dependency);
    bool initializeHWResources();
    void nextStep();
    void updateHWResources(llvm::Instruction* inst);
    unsigned hashInstructionToResourceID(unsigned opcode, unsigned uniqueID) { return (uniqueID  << 6) + (opcode & 0x3f); };
};

}
