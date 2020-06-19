/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 
#include <string> 
#include <list> 
#include <iostream> 
#include <fstream> 

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"


#include "HDLUtil.hpp"
#include "ControlDataInfo.hpp"
#include "HdlObject.hpp"


namespace hdbe {

class HdlObject;
class HdlPort;

using String = std::string;
using Ostream  = std::ostream;

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
    std::ostream& writeCtrlFlow(std::ostream& os );
    String writeHdlObjDeclaration(HdlObject& obj, String tag);
    String writeSimpleInstruction(llvm::Instruction* I);
    String writePHIInstruction(llvm::Instruction* I);
    String writeArrayObject(HdlMemory &array);
    String writeMemoryObject(HdlMemory &memory);
    //String writeCtrlFlowInstruction(llvm::Instruction* I);
    String writeControlActiveCondition(llvm::Instruction* I, llvm::BasicBlock *bb, int id);
    Ostream& writeRegisterStages(Ostream& os);
    Ostream& writeInputAssignment(Ostream& os);
    Ostream& writeReturnStatement(Ostream& os);
    Ostream& writeVCDLogging(Ostream& os);
    Ostream& writeArrayObject(Ostream &os);
};

}
