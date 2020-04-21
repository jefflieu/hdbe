#pragma once 
#include <string> 
#include <list> 


#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace hdbe {

class HardwareDescription {
  
  private: 
    
  
  public: 
    HardwareDescription () {};
    ~HardwareDescription() {};

    float getLatency(const llvm::Instruction* instruction)
    {
      if (instruction->isTerminator()) return 0.0;
      switch(instruction->getOpcode()) 
      {
        case llvm::Instruction::Trunc  :
        case llvm::Instruction::ZExt   :
        case llvm::Instruction::SExt   :
        case llvm::Instruction::Load   : return 0.0;
        case llvm::Instruction::Add    : return 0.2;
        case llvm::Instruction::Sub    : return 0.2;
        case llvm::Instruction::Mul    : return 0.5;
        case llvm::Instruction::Select : return 0.1;
        default: return 1.0;
      }
    }
    float getValidTime(const llvm::Instruction* instruction, float latest_dependency)
    {
      if (instruction->isTerminator()) return latest_dependency;
      switch(instruction->getOpcode()) 
      {
        case llvm::Instruction::Trunc  :
        case llvm::Instruction::ZExt   :
        case llvm::Instruction::SExt   :
        case llvm::Instruction::Load   : return latest_dependency;
        case llvm::Instruction::Add    : return latest_dependency + 0.2;
        case llvm::Instruction::Sub    : return latest_dependency + 0.2;
        case llvm::Instruction::Mul    : return latest_dependency + 0.5;
        case llvm::Instruction::ICmp   : return latest_dependency + 0.1;
        case llvm::Instruction::Select : return latest_dependency + 0.1;
        case llvm::Instruction::Store  : return ceil(latest_dependency + 0.001);
        default: return latest_dependency + 1.0;
      }
    }
};

}
