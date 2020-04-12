#pragma once 

#include "llvm/IR/Value.h"


namespace hdbe {

class HdlObject {
  public: 
    std::string m_name;
    llvm::Value* m_irValue;
    HdlProperty m_property;

  public: 
    HdlObject() { m_name = "noname";}
    HdlObject(std::string name) : m_name(name) {}
    HdlObject(llvm::Value* _irVal) : m_irValue(_irVal) { m_name = m_irValue->getName().str();}
    ~HdlObject() {};    
};


class HdlPort : public HdlObject {
  
  public: 
    HdlPort(std::string name) : HdlObject(name) {};
    HdlPort(llvm::Value* _irVal) : HdlObject(_irVal) {};
    ~HdlPort() {};
 
};

}