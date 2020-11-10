/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include "HardwareDescription.hpp"
#include "DataAnalyzer.hpp"
#include "HdlObject.hpp"

#define HWD_DBG 9

using namespace hdbe;

#define PACK(max, cnt) ((max & 0xffff) << 16 | cnt )
#define UNPACK_MAX(a)  ((a >> 16) & 0xffff)
#define UNPACK_CNT(a)  (a & 0xffff)

HardwareDescription::HardwareDescription () 
{  
  try {  
  YamlNode hwDescription = (YAML::LoadFile("HWDesc.yml")); 
  loadOpcodeInfo(hwDescription);
  }  
  catch(YAML::BadFile & e)
  {
    LOG_S(INFO) << "HWDesc file not found, using default values\n";
    loadDefaultOpcodeInfo();

  }
  catch(YAML::ParserException& e) 
  {    
    LOG_S(INFO) << e.what() << "\n";    
  }
};

bool HardwareDescription::initializeHWResources()
{
  for(auto memObj : CDI_h->memObjList)
  {
    if (memObj.property.vtype == HdlVectorType::memoryType)
    {
      Value *basePtr = memObj.getIrValue();
      unsigned uniqueID = static_cast<unsigned>(reinterpret_cast<uintptr_t>(basePtr));
      this->RM[hashInstructionToResourceID(llvm::Instruction::Load  , uniqueID)] = PACK(1, 1);
      this->RM[hashInstructionToResourceID(llvm::Instruction::Store , uniqueID)] = PACK(1, 1);
      LOG_S(INFO) << "Initializing memory ports for " << basePtr->getName() << "\n";
    }
  }
  return true;
}

void HardwareDescription::nextStep()
{
  for(auto & item: RM)
  {
    unsigned cnt = UNPACK_CNT(item.second);
    unsigned max = UNPACK_MAX(item.second);
    if (cnt < max) cnt++;
    item.second = PACK(max, cnt);
    //LOG_S(INFO) << "Next step " << item.second << "\n";
  }
}

void HardwareDescription::updateHWResources(llvm::Instruction* instruction)
{
  int memOpIdx;
  Value* basePtr;
  unsigned res_id;
  if (isMemoryInstruction(instruction)) {
    basePtr = DataAnalyzer::analyzeMemoryOp(instruction, &memOpIdx);
    unsigned uniqueID = static_cast<unsigned>(reinterpret_cast<uintptr_t>(basePtr));
    res_id  = hashInstructionToResourceID(instruction->getOpcode(), uniqueID); 
    if (RM.count(res_id) > 0) {
      RM[res_id]--;
      LOG_S(HWD_DBG) << "Updated hw resources of " << basePtr->getName() << " " << RM[res_id] << "\n";
    }
  }
}

HardwareDescription::ExecutionInfo HardwareDescription::requestToSchedule(llvm::Instruction* instruction, float latest_dependency)
{
  int memOpIdx;
  Value* basePtr = nullptr;
  ExecutionInfo exeInfo;
  unsigned res_id = 0;
  if (isMemoryInstruction(instruction)) {
    basePtr = DataAnalyzer::analyzeMemoryOp(instruction, &memOpIdx);
    res_id  = hashInstructionToResourceID(instruction->getOpcode(), (unsigned)reinterpret_cast<uintptr_t>(basePtr)); 
    if (instruction->getOpcode() == llvm::Instruction::Load || instruction->getOpcode() == llvm::Instruction::Store)
    {
      if (RM.count(res_id) > 0) {
        exeInfo.hw_available = UNPACK_CNT(RM[res_id]) > 0;
        LOG_S(HWD_DBG) << "Request to schedule " << *instruction << " basePtr " << basePtr << "\n";
        LOG_S(HWD_DBG) << "Resource " << res_id << " availability: " << RM[res_id] << "\n";
      }
    }
  }

  switch(instruction->getOpcode()) 
  {
    case llvm::Instruction::And    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Or     : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::ICmp   : exeInfo.latency = OIM[instruction->getOpcode()].latency; break; 
    case llvm::Instruction::Xor    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Trunc  : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::ZExt   : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::SExt   : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Shl    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::LShr   : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::AShr   : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Add    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Sub    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Mul    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Select : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Load   : exeInfo.latency = (memOpIdx >= 0 ? 0.0 :OIM[instruction->getOpcode()].latency); break;
    case llvm::Instruction::Store  : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::GetElementPtr  : exeInfo.latency = (memOpIdx >= 0 ? 0.0 :OIM[instruction->getOpcode()].latency); break;
    case llvm::Instruction::Ret    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Switch : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::Br     : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
    case llvm::Instruction::PHI    : exeInfo.latency = OIM[instruction->getOpcode()].latency; break;
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

void HardwareDescription::loadOpcodeInfo(YamlNode& hwDescription)
{
  LOG_S(INFO) << "Loading HW Description \n";
  for(int i = 0 ; i < llvm::Instruction::OtherOpsEnd ; i++)
  {
    String opcode = String(llvm::Instruction::getOpcodeName(i));
    if (hwDescription[opcode]) {
      LOG_S(INFO) << "Opcode: " << i << " : " << opcode << "\n";
      LOG_S(INFO) << "  ** Latency     : " << hwDescription[opcode]["latency"].as<float>() << "\n";
      OIM[i] = getOpcodeInfo(hwDescription[opcode]);
    }
  }

}


void HardwareDescription::loadDefaultOpcodeInfo()
{
  LOG_S(INFO) << "Loading default HW Description \n";
  for(int i = 0 ; i < llvm::Instruction::OtherOpsEnd ; i++)
  {
    OpcodeInfo opinfo;
    opinfo.latency      = 0;
    opinfo.input_delay  = 0;
    opinfo.output_delay = 0;
    switch(i) 
    {
      case llvm::Instruction::And    :
      case llvm::Instruction::Or     :
      case llvm::Instruction::ICmp   : 
      case llvm::Instruction::Xor    : opinfo.latency = 0.1; break;
      case llvm::Instruction::Trunc  :
      case llvm::Instruction::ZExt   :
      case llvm::Instruction::SExt   :
      case llvm::Instruction::Shl    :
      case llvm::Instruction::LShr   :
      case llvm::Instruction::AShr   : opinfo.latency = 0.0; break; 
      case llvm::Instruction::Add    : opinfo.latency = 0.33; break;
      case llvm::Instruction::Sub    : opinfo.latency = 0.4; break;
      case llvm::Instruction::Mul    : opinfo.latency = 0.8; break;
      case llvm::Instruction::Select : opinfo.latency = 0.1; break;
      case llvm::Instruction::Load   : opinfo.latency = 1.0; break;
      case llvm::Instruction::Store  : opinfo.latency = 1.0; break;
      case llvm::Instruction::GetElementPtr  : opinfo.latency = 0.0; break;
      case llvm::Instruction::Ret    : opinfo.latency = 0.0; break;
      case llvm::Instruction::Switch : opinfo.latency = 0.1; break;
      case llvm::Instruction::Br     : opinfo.latency = 0.0; break;
      case llvm::Instruction::PHI    : opinfo.latency = 0.1; break;
      default: opinfo.latency = -1.0; break;
    }
    OIM[i] = opinfo;
  }

}

HardwareDescription::OpcodeInfo HardwareDescription::getOpcodeInfo(const YamlNode& opcodeNode)
{
  OpcodeInfo opinfo;
  opinfo.latency       = opcodeNode["latency"].as<float>();
  opinfo.input_delay   = opcodeNode["input_delay"]?opcodeNode["input_delay"].as<float>() : 0.0;
  opinfo.output_delay  = opcodeNode["output_delay"]?opcodeNode["output_delay"].as<float>() : 0.0;
  opinfo.units         = opcodeNode["units"]?opcodeNode["units"].as<unsigned>() : 0;
  return opinfo;
}
