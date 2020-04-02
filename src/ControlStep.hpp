#pragma once 
#include <string> 
#include <list> 

#include "BaseClass.hpp"

namespace hdlbe {

class ControlStep : public BaseClass {
  
  private: 
    bool isBranchNode;
    std::list<ControlStep_h> preds;
    std::list<ControlStep_h> succs;
    std::list<Instruction_h> scheduledInstructions;    
    std::string idString;
    
  
  public: 
    ControlStep (const std::string & id) {this->idString = id;};
    ~ControlStep() {};

    std::string getId() {return this->idString;}

    bool isBranch() {return isBranchNode;};
    
    bool bindInstruction(llvm::Instruction* instruction) {      
      scheduledInstructions.push_back(instruction);
    }
  
};

}
