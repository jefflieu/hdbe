#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "HardwareDescription.hpp"
#include "BaseClass.hpp"
#include "types.hpp"

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DDG.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"


namespace hdlbe {

class SchedulingAlgorithm;

class Scheduler : public BaseClass {
  friend SchedulingAlgorithm;
  protected:     
    Module_h m_irModule;
    std::list<ControlStep_h> m_ctrlSteps;
    llvm::PassBuilder PB;
    llvm::FunctionAnalysisManager FAM;            
    llvm::DataDependenceGraph  *DDG;
    hdlbe::HardwareDescription HWD;

  public: 
    Scheduler (Module_h _irModule) : m_irModule(_irModule) {PB.registerFunctionAnalyses(FAM);};
    ~Scheduler() {delete DDG;};

    virtual uint32_t schedule(SchedulingAlgorithm& algo, std::string funcName){};
    void addControlStep(ControlStep_h cs) {m_ctrlSteps.push_back(cs);}

    void constructDDG(std::string funcName) {
      hdlbe::Function_h F = m_irModule->getFunction(funcName);
      llvm::DependenceAnalysis DA;  
      llvm::DependenceInfo DI   = DA.run(*F, FAM);
      DDG = new llvm::DataDependenceGraph(*F, DI);
      //LLVM_LOG(6, *DDG);
    }
  
};

class SimpleScheduler : public Scheduler {
    
  public:     
    SimpleScheduler (Module_h _irModule) : Scheduler(_irModule) {};
    ~SimpleScheduler() {};

    uint32_t schedule(SchedulingAlgorithm& algo, std::string funcName) override;
  
};


class SchedulingAlgorithm {
  
  public:
    SchedulingAlgorithm () {};
    ~SchedulingAlgorithm () {};
    
    uint32_t visit(SimpleScheduler* scheduler, std::string funcName);
};

}


