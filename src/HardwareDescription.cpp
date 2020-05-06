
#include "HardwareDescription.hpp"

using namespace hdbe;

float HardwareDescription::getLatency(llvm::Instruction* instruction)
{
  if (instruction->isTerminator()) return 0.0;
  if (isMemoryInstruction(instruction)) {


  }
  switch(instruction->getOpcode()) 
  {
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
    default: return 1.0;
  }
}

float HardwareDescription::getValidTime(llvm::Instruction* instruction, float latest_dependency)
{
  if (instruction->isTerminator()) return latest_dependency;
  switch(instruction->getOpcode()) 
  {
    case llvm::Instruction::Store  : return ceil(latest_dependency + 0.001);
    default: return latest_dependency + getLatency(instruction);
  }
}