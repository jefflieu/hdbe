#pragma once 
#include <string> 
#include <list> 
#include <iostream> 
#include <fstream> 

#include "BaseClass.hpp"
#include "Scheduler.hpp"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace hdbe {
class Scheduler;

class CodeGenerator : public BaseClass {  
  
  protected: 
    Scheduler *scheduler;  
    std::string getFunctionName();
    
  public: 
    CodeGenerator() {};
    CodeGenerator(Scheduler *_scheduler): scheduler(_scheduler) {};
    ~CodeGenerator() {};
    std::string makeLegalName(std::string name) {
      std::string invalid_char = ".?+-*";
      std::string new_string;
      for(auto i = name.begin(); i != name.end(); i++)
        {          
          if(invalid_char.find(*i) != std::string::npos)                     
            new_string.push_back('_');
          else 
            new_string.push_back(*i);  
        }
      return new_string;
    }
};

class VhdlGenerator : public CodeGenerator {
  
  public: 
    VhdlGenerator() {};
    VhdlGenerator(Scheduler *_scheduler): CodeGenerator(_scheduler) {};
    ~VhdlGenerator() {};
    
    void write();
  
  private: 
    std::ostream& writeLibrary(std::ostream& os);
    std::ostream& writePorts(std::ostream& os);    
    std::ostream& writeSignalDeclaration(std::ostream& os) {};    
    std::ostream& writeStateSquence(std::ostream& os);        
    std::ostream& writeInstructions(std::ostream& os) {};            
    
};

}
