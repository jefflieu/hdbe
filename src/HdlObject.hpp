#pragma once 

#include "llvm/IR/Value.h"
#include "OSPrint.hpp"

namespace hdbe {

using String = std::string;
using Value  = llvm::Value;

String to_hexstring(uintptr_t, char);

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
        name = "unnamed_" + to_hexstring(reinterpret_cast<uintptr_t>(irValue), 'H');
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