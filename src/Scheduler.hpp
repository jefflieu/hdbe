#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>


#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DDG.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"



#include "types.hpp"
#include "HardwareDescription.hpp"
#include "CodeGenerator.hpp"
#include "ControlStep.hpp"
#include "BaseClass.hpp"
#include "ValueLifeInfo.hpp"

namespace hdbe {

class CodeGenerator;
class VerilogGenerator;
class VhdlGenerator;
class SchedulingAlgorithm;
class ValueLifeInfo;

class Scheduler : public BaseClass {  
  friend CodeGenerator;
  friend VhdlGenerator;
  friend VerilogGenerator;
  friend SchedulingAlgorithm;
  protected:     
    Module_h m_module = nullptr;    
    Function_h m_function = nullptr;    
    std::list<ControlStep> m_ctrlSteps;    
    HardwareDescription HWD;
    std::map<const llvm::Value*, ValueLifeInfo> valueInfoMap;
    

  public: 
    Scheduler (Module_h _irModule) : m_module(_irModule) {};
    ~Scheduler() {};

    virtual uint32_t schedule(SchedulingAlgorithm& algo, std::string funcName){};
    virtual uint32_t schedule(SchedulingAlgorithm& algo, BasicBlock_h bb){};

    //void constructDDG(std::string funcName) {
    //  llvm::PassBuilder PB;
    //  llvm::FunctionAnalysisManager FAM;                
    //  PB.registerFunctionAnalyses(FAM);
    //  m_function = m_module->getFunction(funcName);
    //  llvm::DependenceAnalysis DA;  
    //  llvm::DependenceInfo DI   = DA.run(*m_function, FAM);
    //  DDG = new llvm::DataDependenceGraph(*m_function, DI);
    //  //LLVM_LOG(6, *DDG);
    //}
  
};

class SimpleScheduler : public Scheduler {
    
  public:     
    SimpleScheduler (Module_h _irModule) : Scheduler(_irModule) {};
    ~SimpleScheduler() {};

    uint32_t schedule(SchedulingAlgorithm& algo, std::string funcName) override;
  
};

class BasicBlockScheduler : public Scheduler {
    
  public:     
    BasicBlockScheduler (Module_h _irModule) : Scheduler(_irModule) {};
    ~BasicBlockScheduler() {};

    uint32_t schedule(SchedulingAlgorithm& algo, BasicBlock_h bb) override;
  
};

class SchedulingAlgorithm {
  
  public:
    SchedulingAlgorithm () {};
    ~SchedulingAlgorithm () {};
    
    uint32_t visit(SimpleScheduler* scheduler);
    uint32_t visit(BasicBlockScheduler* scheduler, BasicBlock_h bb);
};

}


