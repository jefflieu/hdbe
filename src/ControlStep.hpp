#pragma once 
#include <string> 
#include <list> 

#include "BaseClass.hpp"

namespace hdbe {

class ControlStep : public BaseClass {
  
  private: 
    bool isBranchNode;
    std::list<ControlStep_h> preds;
    std::list<ControlStep_h> succs;
    std::list<Const_Instruction_h> scheduledInstructions;    
    std::string idString;
    
  
  public: 
    ControlStep (const std::string & id) {this->idString = id;};
    ~ControlStep() {};

    std::string getId() {return this->idString;}

    bool isBranch() {return isBranchNode;};
    
    bool addInstruction(Const_Instruction_h instruction) {      
      scheduledInstructions.push_back(instruction);
    }
  
};

}
