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
#include "VerilogTemplate.hpp"


namespace hdbe {

class HdlObject;
class HdlPort;
class VerilogGenerator;

using String = std::string;
using Ostream  = std::ostream;
using StringRef  = llvm::StringRef;

class CodeGenerator {  

  protected: 
    using HDLIdentifier = String;
    
    class HDLStatement {
      friend VerilogGenerator;
      protected:
        HDLStatement() {};
        ~HDLStatement() {};
        virtual String generateCode(CodeGenerator* codegen) = 0;
    };

    class HDLInstance : HDLStatement {
      friend VerilogGenerator;
      private:
        llvm::SmallVector<HDLIdentifier, 8> params;
        llvm::SmallVector<HDLIdentifier, 8> ports;
        String compName;
        String instName;
      public : 
        HDLInstance() = delete;
        HDLInstance(String component, String instance) : compName(component), instName(instance) {};
        HDLInstance(String component, uint64_t id)     : compName(component), instName(String("I") + llvm::utohexstr(id)) {};
        ~HDLInstance() {}
        void setParam(HDLIdentifier s, int idx = -1)          { if (idx < 0) params.push_back(s); else params[idx] = s; }
        void setParam(int d, int idx = -1)                    { setParam(llvm::itostr(d), idx); }
        void setParam(HDLIdentifier s, int idx_a, int idx_b)  { for(int idx = idx_a; idx < idx_b; idx++) params[idx] = s;}
        void setParam(int d, int idx_a, int idx_b)            { 
                                                                if (idx_b > params.capacity()) params.resize(idx_b);
                                                                setParam(llvm::itostr(d), idx_a, idx_b);
                                                              }
        void setPort(HDLIdentifier s, int idx = -1)           {
                                                                if (idx < 0) ports.push_back(s); else ports[idx] = s;
                                                              }
        void setPort(HDLIdentifier s, int idx_a, int idx_b)   {
                                                                if (idx_b > ports.capacity()) ports.resize(idx_b);
                                                                for(int idx = idx_a; idx < idx_b; idx++) ports[idx] = s;
                                                              }
        void setPortOthers(HDLIdentifier s, int n)            { for(int idx = 0; idx < n; idx++) ports.push_back(s); }
        void setPort(int d, int idx = -1)                     { setPort(llvm::itostr(d), idx); }
        void setPort(int d, int idx_a, int idx_b)             { setPort(llvm::itostr(d), idx_a, idx_b); }
        void setPortOthers(int d, int n)                      { setPortOthers(llvm::itostr(d), n);}
        
        String getInstanceString(CodeGenerator * codegen, bool byName = false) 
        {
          return codegen->visit(this, byName);
        } 

        String generateCode(CodeGenerator * codegen) 
        {
          return codegen->visit(this, false);
        }  
    };

    class HDLCommentLine : public HDLStatement {
      friend VerilogGenerator;
      String data;
      public:
        HDLCommentLine() {};
        HDLCommentLine(String s) : data(s) {};
        ~HDLCommentLine() {};
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLExpression : public HDLStatement {
      friend VerilogGenerator;
      String data;
      public:
        HDLExpression() {};
        HDLExpression(String s) : data(s) {};
        ~HDLExpression() {};
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLAssign : public HDLStatement {
      friend VerilogGenerator;
      protected:
        HDLIdentifier lhs;
        llvm::SmallVector<HDLIdentifier, 8> rhs;
        char op;
        bool seq;
        bool blocking;
      protected: 
        HDLAssign() = delete;
        HDLAssign(HDLIdentifier lhs, char op, bool seq, bool blocking) : lhs(lhs), op(op), seq(seq), blocking(blocking) {};
        ~HDLAssign() {};

        bool isBlocking() {return blocking;}
        bool isSequential() {return seq;}
        char getOperation() {return op;}
        void setRHS(HDLIdentifier oprn) {rhs.push_back(oprn);}

        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLConcurrentAssign : public HDLAssign {
      friend VerilogGenerator;
      public: 
        HDLConcurrentAssign() = delete;
        HDLConcurrentAssign(HDLIdentifier lhs, char op = '\0') : HDLAssign(lhs, op, false, false) {};
        HDLConcurrentAssign(HDLIdentifier lhs, HDLIdentifier rhs, char op = '\0') : HDLAssign(lhs, op, false, false) {setRHS(rhs);};
        ~HDLConcurrentAssign() {};
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLNonblockingAssign : public HDLAssign {
      friend VerilogGenerator;
      public: 
        HDLNonblockingAssign() = delete;
        HDLNonblockingAssign(HDLIdentifier lhs, char op = '\0') : HDLAssign(lhs, op, true, false) {};
        HDLNonblockingAssign(HDLIdentifier lhs, HDLIdentifier rhs, char op = '\0') : HDLAssign(lhs, op, true, false) {setRHS(rhs);};
        ~HDLNonblockingAssign() {};
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLBlockingAssign : public HDLAssign {
      friend VerilogGenerator;
      public: 
        HDLBlockingAssign() = delete;
        HDLBlockingAssign(HDLIdentifier lhs, char op = '\0') : HDLAssign(lhs, op, true, true) {};
        HDLBlockingAssign(HDLIdentifier lhs, HDLIdentifier rhs, char op = '\0') : HDLAssign(lhs, op, true, true) {setRHS(rhs);};
        ~HDLBlockingAssign() {};
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLIfThenElse : public HDLStatement {
      friend VerilogGenerator;
      protected:
        String cond;
        llvm::SmallVector<HDLStatement*, 8> then_statements;
        llvm::SmallVector<HDLStatement*, 8> else_statements;
      public: 
        HDLIfThenElse() = delete;
        HDLIfThenElse(String cond) : cond(cond) {};
        ~HDLIfThenElse() {then_statements.clear(); else_statements.clear();};
        
        HDLStatement* addThenStatement(HDLStatement* statement) {then_statements.push_back(statement);}
        HDLStatement* addElseStatement(HDLStatement* statement) {else_statements.push_back(statement);}

        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
    };

    class HDLSequentialBlock : public HDLStatement {
      friend VerilogGenerator;
      protected:
        bool comb   ;
        HDLIdentifier clock;
        llvm::SmallVector<HDLStatement*, 8> statements;
        String name;
      public: 
        HDLSequentialBlock() : comb(false), clock("") {};
        HDLSequentialBlock(String name) : name(name), comb(false), clock("") {};
        HDLSequentialBlock(String name, HDLIdentifier clock_net) : name(name), clock(clock_net), comb(false) {};
        ~HDLSequentialBlock() {statements.clear();};
        
        HDLStatement* addStatement(HDLStatement *statement) {statements.push_back(statement); return statement;}
        bool isCombinatorial() { return comb;}
        void setCombinatorial(bool c) {comb = c;} 
        bool isClocked() { return !clock.empty();}
        StringRef getName() { return StringRef(name);}
        StringRef getClock() { return StringRef(clock);}
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
        void delete_statements() {statements.clear();}
    };

    class HDLConcurrentBlock : public HDLStatement {
      friend VerilogGenerator;
      protected:
        llvm::SmallVector<HDLStatement*, 8> statements;
        String name;
      public: 
        
        HDLConcurrentBlock() : name("") {};
        HDLConcurrentBlock(String name) : name(name) {};
        ~HDLConcurrentBlock() {statements.clear();};
        
        HDLStatement* addStatement(HDLStatement *statement) {statements.push_back(statement); return statement;}
        StringRef getName() { return StringRef(name);}
        String generateCode(CodeGenerator * codegen) {return codegen->visit(this);} 
        void delete_statements() {statements.clear();}
    };

  protected:     
    ControlDataInfo *CDI_h;
    
  public: 
    CodeGenerator() {};
    CodeGenerator(ControlDataInfo *_CDI_h) : CDI_h(_CDI_h) {};
    ~CodeGenerator() {};    
    virtual String visit(HDLInstance * inst, bool byName) = 0;
    virtual String visit(HDLAssign * sm) = 0;
    virtual String visit(HDLIfThenElse * sm) = 0;
    virtual String visit(HDLSequentialBlock * seq) = 0;
    virtual String visit(HDLConcurrentBlock * con) = 0;
    virtual String visit(HDLCommentLine * con) = 0;
    virtual String visit(HDLExpression* expr) = 0;
};

class VerilogGenerator : public CodeGenerator {
   using HDLIndentififer = String; 
  public: 
    VerilogGenerator() {};
    VerilogGenerator(ControlDataInfo *_CDI_h) : CodeGenerator(_CDI_h) {}
    ~VerilogGenerator() {}
    
    void write();
  
  private: 
    
    std::ostream& writePorts(std::ostream& os);    
    std::ostream& writeSignalDeclaration(std::ostream& os);    
    //std::ostream& writeStateSquence(std::ostream& os); deprecated by writeCtrlFlow       
    std::ostream& writeInstructions(std::ostream& os);
    std::ostream& writeCtrlFlow(std::ostream& os );
    String writeHdlObjDeclaration(HdlObject& obj, String tag = "");
    String writeSimpleInstruction(llvm::Instruction* I);
    String writePHIInstruction(llvm::Instruction* I);
    String writeArrayObject(HdlMemory &array);
    String writeMemoryObject(HdlMemory &memory);
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

    String visit(HDLAssign* statement) override {
      String text;
      if (!statement->isSequential()) text += VERILOG_ASSIGN_STATEMENT;
      text += statement->lhs;
      text += (!statement->isSequential() || statement->isBlocking())?VERILOG_CONT_ASSIGN:VERILOG_ASSIGN;
      for(int i = 0; i < statement->rhs.size(); i++)
      {
        if (i > 0) text += String(1,statement->op);
        text += statement->rhs[i];
      }
      text += VERILOG_ENDL;
      return text;
    };

    String visit(HDLIfThenElse* ifthenelse) override {
      String text;
      text += VERILOG_IF(ifthenelse->cond);
      if (ifthenelse->then_statements.size() > 1) text += VERILOG_BEGIN;
      for(auto s : ifthenelse->then_statements)
      {
        text += s->generateCode(this);
      }
      if (ifthenelse->then_statements.size() > 1) text += VERILOG_END;
      if (ifthenelse->else_statements.size() > 0) text += VERILOG_ELSE;
      if (ifthenelse->else_statements.size() > 1) text += VERILOG_BEGIN;
      for(auto s : ifthenelse->else_statements)
      {
        text += s->generateCode(this);
      }
      if (ifthenelse->else_statements.size() > 1) text += VERILOG_END;
      
      return text;
    };

    String visit(HDLSequentialBlock* seqBlock) override {
      String text;
      text += "\n";
      if (seqBlock->isClocked())
        text += VERILOG_ALWAYS_CLK_BLK(seqBlock->getName().str(), seqBlock->getClock().str());
      else if (seqBlock->isCombinatorial())
        text += VERILOG_ALWAYS_COMB_BLK(seqBlock->getName().str());
      else 
        text += VERILOG_BEGIN; 
      for(auto s : seqBlock->statements) 
      {
        text += s->generateCode(this);
      }
      text += VERILOG_END;
      text += "\n";
      return text;
    };
    
    String visit(HDLConcurrentBlock* conBlock) override {
      String text;
      text += VERILOG_CODE_SECTION(conBlock->getName().str()); 
      for(auto s : conBlock->statements) 
      {
        text += s->generateCode(this);
      }
      text += VERILOG_COMMENT + conBlock->getName().str() + " end\n"; 
      return text;
    };

    String visit(HDLCommentLine* comment) override {
      String text;
      text += VERILOG_COMMENT + comment->data + "\n";
      return text;
    };

    String visit(HDLExpression* expr) override {
      return expr->data;
    };

    
};

}
