#pragma once 

#include "llvm/IR/Value.h"
#include "OSPrint.hpp"

namespace hdbe {

std::string to_hexstring(uintptr_t, char);

class HdlObject {
  public: 
    std::string m_name;
    llvm::Value* m_irValue;
    HdlProperty m_property;

  public: 
    HdlObject() { m_name = "noname";}
    HdlObject(std::string name) : m_name(name) {}
    HdlObject(llvm::Value* _irVal) : m_irValue(_irVal) {
      if (m_irValue->hasName())
        m_name = m_irValue->getName().str();
      else 
        m_name = "unnamed_" + to_hexstring(reinterpret_cast<uintptr_t>(m_irValue), 'H');
    }
    ~HdlObject() {};    
};


class HdlPort : public HdlObject {
  
  public: 
    HdlPort(std::string name) : HdlObject(name) {};
    HdlPort(llvm::Value* _irVal) : HdlObject(_irVal) {};
    ~HdlPort() {};
 
};


class HdlVariable : public HdlObject {
  
  public: 
    HdlVariable(std::string name) : HdlObject(name) {};
    HdlVariable(llvm::Value* _irVal) : HdlObject(_irVal) {};
    ~HdlVariable() {};
 
};

}