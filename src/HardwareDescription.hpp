#pragma once 
#include <string> 
#include <list> 

#include "BaseClass.hpp"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace hdlbe {

class HardwareDescription : public BaseClass {
  
  private: 
    
  
  public: 
    HardwareDescription () {};
    ~HardwareDescription() {};

    float getLatency(const llvm::Instruction* instruction)
    {
      switch(instruction->getOpcode()) 
      {
        case llvm::Instruction::Load : return 0.0;
        default: return 1.0;
      }
    }
};

}
