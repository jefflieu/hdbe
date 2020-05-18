
#include "HardwareDescription.hpp"

using namespace hdbe;

float HardwareDescription::getLatency(llvm::Instruction* instruction)
{
  if (isMemoryInstruction(instruction)) {


  }
  switch(instruction->getOpcode()) 
  {
    case llvm::Instruction::And    :
    case llvm::Instruction::Or     :
    case llvm::Instruction::ICmp   : 
    case llvm::Instruction::Xor    : return 0.1;
    case llvm::Instruction::Trunc  :
    case llvm::Instruction::ZExt   :
    case llvm::Instruction::SExt   :
    case llvm::Instruction::Shl    :
    case llvm::Instruction::LShr   :
    case llvm::Instruction::AShr   : return 0.0;
    case llvm::Instruction::Add    : return 0.4;
    case llvm::Instruction::Sub    : return 0.4;
    case llvm::Instruction::Mul    : return 0.8;
    case llvm::Instruction::Select : return 0.1;
    case llvm::Instruction::Load   : return 0.0;
    case llvm::Instruction::Store  : return 1.0;
    case llvm::Instruction::GetElementPtr  : return 0.0;
    case llvm::Instruction::Ret    : return 0.0;
    case llvm::Instruction::Switch : return 0.1;
    case llvm::Instruction::Br     : return 0.1;
    case llvm::Instruction::PHI    : return 0.1;
    default: return 1.0;
  }
}

float HardwareDescription::getValidTime(llvm::Instruction* instruction, float latest_dependency)
{
  float lat = getLatency(instruction);
  switch(instruction->getOpcode()) 
  {
    case llvm::Instruction::Ret    :  
    case llvm::Instruction::Switch :
    case llvm::Instruction::Br     :  return latest_dependency + lat;                                      
    case llvm::Instruction::Store  :  return ceil(latest_dependency + 0.001);
    default: return latest_dependency + getLatency(instruction);
  }
}