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
class VerilogGenerator;

using String = std::string;
using Ostream  = std::ostream;

class CodeGenerator {  
  public: 
    class HDLInstance {
      friend VerilogGenerator;
      private:
        llvm::SmallVector<String, 8> params;
        llvm::SmallVector<String, 8> ports;
        String compName;
        String instName;
      public : 
        HDLInstance() = delete;
        HDLInstance(String component, String instance) : compName(component), instName(instance) {};
        HDLInstance(String component, uint64_t id)     : compName(component), instName(String("I") + llvm::utohexstr(id)) {};
        ~HDLInstance() {}
        void setParam(String s, int idx = -1) {if (idx < 0) params.push_back(s); else params[idx] = s;}
        void setParam(int d, int idx = -1) {setParam(llvm::itostr(d), idx);}
        void setParam(String s, int idx_a, int idx_b) {for(int idx = idx_a; idx < idx_b; idx++) params[idx] = s;}
        void setParam(int d, int idx_a, int idx_b) {
            if (idx_b > params.capacity()) params.resize(idx_b);
            setParam(llvm::itostr(d), idx_a, idx_b);}
        void setPort(String s, int idx = -1) {if (idx < 0) ports.push_back(s); else ports[idx] = s;}
        void setPort(String s, int idx_a, int idx_b) {
            if (idx_b > ports.capacity()) ports.resize(idx_b);
            for(int idx = idx_a; idx < idx_b; idx++) ports[idx] = s;}
        void setPortOthers(String s, int n) {
            for(int idx = 0; idx < n; idx++) ports.push_back(s);}
        void setPort(int d, int idx = -1) {setPort(llvm::itostr(d), idx);}
        void setPort(int d, int idx_a, int idx_b) {setPort(llvm::itostr(d), idx_a, idx_b);}
        void setPortOthers(int d, int n) {setPortOthers(llvm::itostr(d), n);}
        String getInstanceString(CodeGenerator * codegen, bool byName = false) {return codegen->visit(this, byName);};  
  };

  protected:     
    ControlDataInfo *CDI_h;
    
  public: 
    CodeGenerator() {};
    CodeGenerator(ControlDataInfo *_CDI_h) : CDI_h(_CDI_h) {};
    ~CodeGenerator() {};    
    virtual String visit(HDLInstance * inst, bool byName) = 0;
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
    
    
    String visit(HDLInstance* inst, bool byname = false) override {
      String instance;
      int size;
      char delimit;
      char buf[256];
      //Write Address mux
      size = sprintf(buf,"%10s #(", inst->compName.data());
      instance += String(buf, size); 
      for(int i = 0; i < inst->params.size(); i++)    
      {
        delimit  = (i == (inst->params.size()-1))?' ':',';
        size = sprintf(buf," %10s%c", inst->params[i].data(), delimit);
        instance += String(buf, size); 
      }
      
      size = sprintf(buf,") %16s ( ", inst->instName.data());
      instance += String(buf, size);
      for(int i = 0; i < inst->ports.size(); i++)    
      {
        delimit  = (i == (inst->ports.size()-1))?' ':',';
        size = sprintf(buf," %6s%c", inst->ports[i].data(), delimit);
        instance += String(buf, size); 
      }
      
      instance += ");\n";
      return instance;
    };

    
};

}
