#pragma once 
#include <string> 
#include <list> 

#include "BaseClass.hpp"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace hdbe {

class HardwareDescription : public BaseClass {
  
  private: 
    
  
  public: 
    HardwareDescription () {};
    ~HardwareDescription() {};

    float getLatency(const llvm::Instruction* instruction)
    {
      if (instruction->isTerminator()) return 0.0;
      switch(instruction->getOpcode()) 
      {
        case llvm::Instruction::Load : return 0.0;
        default: return 1.0;
      }
    }
};

}
