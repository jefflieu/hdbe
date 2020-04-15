#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "HardwareDescription.hpp"
#include "CodeGenerator.hpp"
#include "ControlStep.hpp"
#include "BaseClass.hpp"
#include "types.hpp"

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DDG.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadicDetails.h"
#include "llvm/Support/FormatVariadic.h"

namespace hdbe {

class ControlStep : public BaseClass {
  
  public: 
    bool m_isBranch = false;
    bool m_isRet = false;
    std::list<const llvm::Instruction *> m_instrList;    
    std::list<ControlStep *> m_outList;    
    std::list<ControlStep *> m_inList;    
    llvm::BasicBlock *m_bbHandle;    
    std::string m_bbName;    
    int m_id = 0;
  
  public: 
    ControlStep (llvm::BasicBlock *bb, int id) :  m_bbHandle(bb), m_id(id) {m_bbName = m_bbHandle->getName().str();}
    ~ControlStep() {}

    const std::string& getbbName() {return this->m_bbName;}
    
    int  getId() {return this->m_id;}
    bool isBranch() {return m_isBranch;};
    void setBranch(bool branch) {m_isBranch = branch;};
    void setId(int _id) {m_id = _id;}
    void setReturn(bool ret) {m_isRet = ret;}
    bool isEntry() {return (m_id == 0);}
    bool isReturn() {return m_isRet;}
    
    bool addInstruction(const llvm::Instruction * instruction) {      
      m_instrList.push_back(instruction);
    }
    
    
    

    std::ostream& print(std::ostream& os) const {            
      os << "Step: " << m_bbName << m_id << " branch: " << m_isBranch <<'\n';
      for(auto I = m_instrList.begin(); I != m_instrList.end(); ++I){   
        std::string name = g_getStdStringName(*I);          
        os << (*I) << " " << name << " " << (*I)->getOpcodeName() << '\n'; 
      }
      return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const ControlStep& cs);    
};

}
