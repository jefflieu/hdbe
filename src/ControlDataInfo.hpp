/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#pragma once 

#include <typeinfo> 

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"


#include "logging/logger.hpp"
#include "HdlObject.hpp"
#include "HdlState.hpp"
#include "ValueLifeInfo.hpp"
#include "HardwareDescription.hpp"
#include "DependenceInfo.hpp"


namespace hdbe {

using CFGEdgeVector = llvm::SmallVector<HdlCFGEdge*, 16>;


/// This class holds all the results performed by other blocks such as DataAnalyzer, InstructionScheduler ..
/// The results are stored in various lists which serve the ultimate goal of creating HDL description of the IR

class ControlDataInfo {
  

  using Module      = llvm::Module;
  using Function    = llvm::Function;
  using Instruction = llvm::Instruction;
  using Value       = llvm::Value;
  using BasicBlock  = llvm::BasicBlock;
  using Hashcode    = llvm::hash_code;
  using DataLayout  = llvm::DataLayout;
  using LoopInfo    = llvm::LoopInfo;
  using MemorySSA   = llvm::MemorySSA;
  
  friend class DataAnalyzer;
  friend class VerilogGenerator;
  friend class InstructionScheduler;
  friend class HardwareDescription;

  protected: 
    Module   *irModule   = nullptr; 
    Function *irFunction = nullptr;
    

    std::list<HdlPort     > portList;
    std::list<HdlVariable > variableList;
    std::list<HdlMemory   > memObjList;
    std::list<HdlState    > stateList;    
    std::list<HdlCFGEdge  > transitionList;    
    std::map<Hashcode, HdlCFGEdge*> edgeMap;    
    std::map<Value*, ValueLifeInfo> valueInfoMap;
    std::map<Value*, DependenceInfo> dependencyMap;
    HardwareDescription     HWD;


    //Construct Pass builder 
    llvm::PassBuilder PB;
    llvm::FunctionPassManager FPM;
    llvm::FunctionAnalysisManager FAM;
    llvm::LoopPassManager LPM;
    llvm::LoopAnalysisManager LAM;

  public :
    static char ID;
    
    explicit ControlDataInfo (Module *_module, const char* functionName) : irModule(_module) 
      { 
        this->irFunction = irModule->getFunction(functionName);
        if (!irFunction) 
          LOG(ERROR, "Function named " << functionName << " .. not found");
        else {
          LOG(INFO, "Module loaded");

          FPM = llvm::FunctionPassManager(true);
          FAM = llvm::FunctionAnalysisManager(true);
          PB.registerFunctionAnalyses(FAM);
          FPM.run(*irFunction, FAM);
                    
        }
        HWD.setParent(this);
      }

    ~ControlDataInfo () {}

    inline auto addValueInfo (Value* val) {
      return valueInfoMap.insert(std::pair<Value*, ValueLifeInfo>(val, ValueLifeInfo(val)));
    }

    inline auto addDependenceInfo(Value* val) {
      return dependencyMap.insert(std::pair<Value*, DependenceInfo>(val, DependenceInfo(val)));
    }

    HdlState&   getInstructionState(Instruction* I);
    HdlCFGEdge& findCFGEdge(BasicBlock* src, BasicBlock* dst);
    CFGEdgeVector findAllCFGEdges(BasicBlock* src, BasicBlock* dst);
    void        addCFGEdge(HdlCFGEdge& edge);
    unsigned    getPointerSizeInBits() {const DataLayout & DL = this->irModule->getDataLayout(); return DL.getPointerSizeInBits();} 
    
    llvm::LoopInfo & getLoopInfo()  {return FAM.getResult<llvm::LoopAnalysis>(*irFunction);}
    llvm::MemorySSA& getMemorySSA() {return FAM.getResult<llvm::MemorySSAAnalysis>(*irFunction).getMSSA();}
    llvm::MemoryDependenceResults & getMemoryDependenceResults() {return FAM.getResult<llvm::MemoryDependenceAnalysis>(*irFunction);}
    llvm::DominatorTree & getDominatorTree() {return FAM.getResult<llvm::DominatorTreeAnalysis>(*irFunction);}
    llvm::DependenceInfo & getDependenceInfo() {return FAM.getResult<llvm::DependenceAnalysis>(*irFunction);}
    void dumpStateList();
    void dumpValueInfoMap();
    void dumpDependencyMap();
    
};


}

