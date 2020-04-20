#pragma once 
#include <string> 
#include <list> 
#include <iostream> 
#include <fstream> 

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"


#include "HDLUtil.hpp"
#include "ControlDataInfo.hpp"
#include "HdlObject.hpp"


namespace hdbe {

class HdlObject;
class HdlPort;

class CodeGenerator {  
  
  protected:     
    ControlDataInfo *CDI_h;
    
  public: 
    CodeGenerator() {};
    CodeGenerator(ControlDataInfo *_CDI_h) : CDI_h(_CDI_h) {};
    ~CodeGenerator() {};    
};

class VerilogGenerator : public CodeGenerator {
  
  public: 
    VerilogGenerator() {};
    VerilogGenerator(ControlDataInfo *_CDI_h) : CodeGenerator(_CDI_h) {}
    ~VerilogGenerator() {}
    
    void write();
  
  private: 
    
    std::ostream& writePorts(std::ostream& os);    
    std::ostream& writeSignalDeclaration(std::ostream& os);    
    std::ostream& writeStateSquence(std::ostream& os);        
    std::ostream& writeInstructions(std::ostream& os);
    std::string writeHdlObjDeclaration(HdlObject& obj);
    std::string writeOneInstruction(const llvm::Instruction* I);
};

}
