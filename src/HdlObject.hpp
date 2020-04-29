#pragma once 
#include <list>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include "HDLUtil.hpp"

namespace hdbe {

using String = std::string;
using Value  = llvm::Value;
using Instruction = llvm::Instruction;
using String = std::string;
using InstrucionList = std::list<Instruction*>;

enum class HdlVectorType {scalarType, arrayType, memoryType};
enum class HdlSignalType {combType, regType, inputType, outputType};

struct HdlProperty {
  HdlVectorType vtype = HdlVectorType::scalarType;
  HdlSignalType stype = HdlSignalType::combType;
  int  bitwidth = 0;
  int  arraylength = 0;
  bool isConstant = false;
  int64_t dflt = 0;
};


class HdlObject {
  private:
    Value* irValue;

  public: 
    String name;
    HdlProperty property;

  public: 
    HdlObject() { name = "noname";}
    HdlObject(String name) : name(name) {}
    HdlObject(Value* _irVal) : irValue(_irVal) {
      name = getValueHdlName(irValue);
    }
    
    Value* getIrValue() {return irValue;}
    
    ~HdlObject() {};    
};


class HdlPort : public HdlObject {
  
  public: 
    HdlPort(String name) : HdlObject(name) {};
    HdlPort(Value* _irVal) : HdlObject(_irVal) {};
    ~HdlPort() {};
 
};


class HdlVariable : public HdlObject {
  
  public: 
    HdlVariable(String name) : HdlObject(name) {};
    HdlVariable(Value* _irVal) : HdlObject(_irVal) {};
    ~HdlVariable() {};
 
};

class HdlMemory : public HdlObject {
  
  public: 
    HdlMemory(String name) : HdlObject(name) {};
    HdlMemory(Value* _irVal) : HdlObject(_irVal) {};
    ~HdlMemory() {};
    InstrucionList memInstrList;
 
};

}