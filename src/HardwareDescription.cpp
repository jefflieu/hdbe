
#include "HardwareDescription.hpp"
#include "DataAnalyzer.hpp"

using namespace hdbe;


HardwareDescription::ExecutionInfo HardwareDescription::requestToSchedule(llvm::Instruction* instruction, float latest_dependency)
{
  int memOpIdx;
  Value* basePtr;
  ExecutionInfo exeInfo;
  if (isMemoryInstruction(instruction)) {
    basePtr = DataAnalyzer::analyzeMemoryOp(instruction, &memOpIdx);
  }
  switch(instruction->getOpcode()) 
  {
    case llvm::Instruction::And    :
    case llvm::Instruction::Or     :
    case llvm::Instruction::ICmp   : 
    case llvm::Instruction::Xor    : exeInfo.latency = 0.1; break;
    case llvm::Instruction::Trunc  :
    case llvm::Instruction::ZExt   :
    case llvm::Instruction::SExt   :
    case llvm::Instruction::Shl    :
    case llvm::Instruction::LShr   :
    case llvm::Instruction::AShr   : exeInfo.latency = 0.0; break; 
    case llvm::Instruction::Add    : exeInfo.latency = 0.33; break;
    case llvm::Instruction::Sub    : exeInfo.latency = 0.4; break;
    case llvm::Instruction::Mul    : exeInfo.latency = 0.8; break;
    case llvm::Instruction::Select : exeInfo.latency = 0.1; break;
    case llvm::Instruction::Load   : exeInfo.latency = (memOpIdx >= 0 ? 0.0 :1.0); break;
    case llvm::Instruction::Store  : exeInfo.latency = 1.0; break;
    case llvm::Instruction::GetElementPtr  : exeInfo.latency = (memOpIdx >= 0 ? 0.0 :0.0); break;
    case llvm::Instruction::Ret    : exeInfo.latency = 0.0; break;
    case llvm::Instruction::Switch : exeInfo.latency = 0.1; break;
    case llvm::Instruction::Br     : exeInfo.latency = 0.1; break;
    case llvm::Instruction::PHI    : exeInfo.latency = 0.1; break;
    default: exeInfo.latency = -1.0; break;
  }

  switch(instruction->getOpcode()) 
  {
    case llvm::Instruction::Load   :  if (memOpIdx >= 0) {
                                        exeInfo.valid    = latest_dependency; 
                                        exeInfo.pipeline = 0; 
                                        break;
                                      }
    case llvm::Instruction::Store  :  exeInfo.valid = ceil(latest_dependency + 0.001);
                                      exeInfo.pipeline = 1;
                                      break;
    default:  exeInfo.valid = latest_dependency + exeInfo.latency;
              exeInfo.pipeline = 0;
              break;
  }

  return exeInfo;
}