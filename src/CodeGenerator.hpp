#pragma once 
#include <string> 
#include <list> 
#include <iostream> 
#include <fstream> 

#include "BaseClass.hpp"
#include "CodeGenerator.hpp"
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
    std::ostream& writeStateSquence(std::ostream& os) {};        
    std::ostream& writeInstructions(std::ostream& os) {};            
    
};

}
