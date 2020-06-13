#pragma once 
#include <list>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/StringRef.h"

#include "HDLUtil.hpp"
#include "logging/logger.hpp"

namespace hdbe {

using String = std::string;
using Value  = llvm::Value;
using Instruction = llvm::Instruction;
using String = std::string;
using InstrucionList = std::list<Instruction*>;
using StringRef = llvm::StringRef;
using BasicBlock = llvm::BasicBlock;

enum class HdlVectorType {scalarType, arrayType, memoryType};
enum class HdlSignalType {combType, regType, inputType, outputType, inoutType};

struct HdlProperty {
  HdlVectorType vtype = HdlVectorType::scalarType;
  HdlSignalType stype = HdlSignalType::combType;
  int  bitwidth = 0;
  int  arraylength = 0;
  bool isConstant = false;
  bool isUnused = false;
  bool isBackValue = false;
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
    StringRef getName() {return StringRef(name);}
    
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
  unsigned addrSize   = 64;
  unsigned memorySize = 0;
  public: 
    HdlMemory(String name) : HdlObject(name) {}
    HdlMemory(Value* _irVal) : HdlObject(_irVal) {addrSize = 64;}
    ~HdlMemory() {};
    InstrucionList memInstrList;
    unsigned getAddrSize() {return addrSize;}
    unsigned getMemorySize() {return memorySize;}
    void setAddrSize(unsigned s) {addrSize = s;}
    void setMemorySize(unsigned s) {memorySize = s;}
};

class HdlCFGEdge : public HdlObject {
  int successorId = 0; //The Successor ID 
  public: 
    HdlCFGEdge(String name) : HdlObject(name) {};
    HdlCFGEdge(Value* _irVal, int _id ) : HdlObject(_irVal), successorId(_id) {name += "_br" + std::to_string(_id);};
    ~HdlCFGEdge() {};

    //simple back_edge check 
    bool isBackEdge(){return property.isBackValue || (getSrcBB() == getDestBB());}

    BasicBlock* getSrcBB() {return static_cast<Instruction*>(this->getIrValue())->getParent();}
    BasicBlock* getDestBB() { BasicBlock* successor = static_cast<Instruction*>(this->getIrValue())->getSuccessor(successorId); 
                              return successor;}
};


}