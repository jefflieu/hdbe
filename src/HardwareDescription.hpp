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

#include "yaml-cpp/yaml.h"
#include "logging/logger.hpp"

#include "IRUtil.hpp"



namespace hdbe {


class ControlDataInfo;
class HardwareDescription {  

  struct OpcodeInfo
    {
       float latency      = 0.0;
       float input_delay  = 0.0;
       float output_delay = 0.0;
       unsigned units     = 0;
    };
  
  using ResourceMap = std::map<unsigned, unsigned>;
  using YamlNode    = YAML::Node;
  using String      = std::string;
  using OpcodeInfoMap = std::map<unsigned, OpcodeInfo>;
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
    //YamlNode YmlHWDes;
    OpcodeInfoMap OIM;

  public: 
    HardwareDescription ();
    ~HardwareDescription() {};

    void setParent(ControlDataInfo* cdi) {CDI_h = cdi;}
    ExecutionInfo requestToSchedule(llvm::Instruction* instruction, float latest_dependency);
    bool initializeHWResources();
    void nextStep();
    void updateHWResources(llvm::Instruction* inst);
    unsigned hashInstructionToResourceID(unsigned opcode, unsigned uniqueID) { return (uniqueID  << 6) + (opcode & 0x3f); };
    void loadOpcodeInfo(YamlNode& ymlHwDescription);
    void loadDefaultOpcodeInfo();
    OpcodeInfo getOpcodeInfo(const YamlNode& opcodeNode);
};



}
