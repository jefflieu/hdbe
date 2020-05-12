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


namespace hdbe {
  
using Instruction = llvm::Instruction;
using BasicBlock  = llvm::BasicBlock;
using StringRef   = llvm::StringRef;
using String      = std::string;

class HdlState : public HdlObject {
  
  public: 
    std::list<Instruction* > instructionList;        
    Instruction* termInstruction = nullptr;
    BasicBlock * block = nullptr;        
    int id = 0;
    String name = "state_00";
    String empty_name = "";
    bool is_last = false;
  
  public: 
    HdlState (BasicBlock *bb, int _id) :  block(bb), id(_id) { name = "state_" + block->getName().str() + std::to_string(id);}
    HdlState (int _id) :  block(nullptr), id(_id) { name = "state_" + std::to_string(id);}
    ~HdlState() {}

    StringRef   getbbName() {if (block) return block->getName(); else return StringRef(empty_name);}
    String      getbbNameStr() {if (block) return block->getName().str(); else return String("");}
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
