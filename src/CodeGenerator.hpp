#pragma once 
#include <string> 
#include <list> 
#include <iostream> 
#include <fstream> 

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"


#include "HDLUtil.hpp"
#include "BaseClass.hpp"
#include "Scheduler.hpp"
#include "DataAnalyzer.hpp"
#include "HdlObject.hpp"


namespace hdbe {
class Scheduler;
class DataAnalyzer;
class HdlObject;
class HdlPort;

class CodeGenerator : public BaseClass {  
  
  protected: 
    Scheduler *scheduler;  
    DataAnalyzer *analyzer;  
    std::string getFunctionName();
    
  public: 
    CodeGenerator() {};
    CodeGenerator(Scheduler *_scheduler, DataAnalyzer *_analyzer): scheduler(_scheduler), analyzer(_analyzer) {};
    ~CodeGenerator() {};    
};

class VhdlGenerator : public CodeGenerator {
  
  public: 
    VhdlGenerator() {};
    VhdlGenerator(Scheduler *_scheduler, DataAnalyzer* _analyzer): CodeGenerator(_scheduler, _analyzer){};
    ~VhdlGenerator() {};
    
    void write();
  
  private: 
    std::ostream& writeLibrary(std::ostream& os);
    std::ostream& writePorts(std::ostream& os);    
    std::ostream& writeSignalDeclaration(std::ostream& os) {};    
    std::ostream& writeStateSquence(std::ostream& os);        
    std::ostream& writeInstructions(std::ostream& os) {};            
    
};

class VerilogGenerator : public CodeGenerator {
  
  public: 
    VerilogGenerator() {};
    VerilogGenerator(Scheduler *_scheduler, DataAnalyzer* _analyzer): CodeGenerator(_scheduler, _analyzer){};
    ~VerilogGenerator() {};
    
    void write();
  
  private: 
    
    std::ostream& writePorts(std::ostream& os);    
    std::ostream& writeSignalDeclaration(std::ostream& os);    
    std::ostream& writeStateSquence(std::ostream& os);        
    std::ostream& writeInstructions(std::ostream& os);
    std::string writeHdlObjDeclaration(HdlObject& obj);
    std::string writeOneInstruction(const llvm::Instruction* I, ControlStep& s);
};

}
