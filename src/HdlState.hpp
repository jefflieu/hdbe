#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"

#include "HdlObject.hpp"
#include "HDLUtil.hpp"


namespace hdbe {
  
using Instruction = llvm::Instruction;
using BasicBlock  = llvm::BasicBlock;
using StringRef   = llvm::StringRef;
using String      = std::string;

class HdlState : public HdlObject {

  using InstListType = std::list<Instruction*>;
  
  public: 
    InstListType instructionList;        
    Instruction* termInstruction = nullptr;
    int id = 0;
    String name = "state_00";
    String empty_name = "";
    bool is_last = false;
  
  public: 
    HdlState (int _id) :  id(_id) { name = makeHdlStateName(id);}
    ~HdlState() {}

    StringRef   getName() {return StringRef(name);}
        
    int   getId() {return this->id;}
    bool  isBranch() {return (termInstruction!=nullptr) || is_last;}
    void  setId(int _id) {id = _id;}
    bool  isEntry() {return (id == 0);}
    bool  isReturn() { if (!termInstruction) return is_last; 
                       return (termInstruction->getOpcode() == llvm::Instruction::Ret);} 
    void  isLast(bool tf)  {is_last = tf;}
    void  dump(){
      _log_stdout << name << "\n";
      for(Instruction* i : instructionList)
      {
        _log_stdout<<"Instruction: " << (*i) << "\n";
      }
      if (termInstruction)
        _log_stdout<<"Terminator : " << (*termInstruction) << "\n";
      if (is_last)
        _log_stdout<<"** last state **\n";
    }                     
};

}
