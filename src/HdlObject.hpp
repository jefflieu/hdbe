#pragma once 

#include "llvm/IR/Value.h"
#include "llvm/ADT/StringExtras.h"

#include "types.hpp"

namespace hdbe {

using String = std::string;
using Value  = llvm::Value;

enum class HdlVectorType {scalarType, arrayType, memoryType};
enum class HdlSignalType {combType, regType, inputType, outputType};

struct HdlProperty {
  HdlVectorType vtype = HdlVectorType::scalarType;
  HdlSignalType stype = HdlSignalType::combType;
  int  bitwidth = 0;
  int  arraylength = 0;
  bool isConstant = false;
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
      if (irValue->hasName())
        name = irValue->getName().str();
      else 
        name = "unnamed_" + llvm::utohexstr(reinterpret_cast<uintptr_t>(irValue));
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

}