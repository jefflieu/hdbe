#pragma once 

#include <typeinfo> 

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

    HardwareDescription     HWD;

  public :
    ControlDataInfo () {HWD.setParent(this);};     
    ControlDataInfo (Module *_module, const char* functionName) : irModule(_module) 
      { 
        this->irFunction = irModule->getFunction(functionName);
        if (!irFunction) 
          LOG(ERROR, "Function named " << functionName << " .. not found");
        else 
          LOG(INFO, "Module loaded");  
      }
    ~ControlDataInfo () {}

    inline auto addValueInfo (Value* val) {
      return valueInfoMap.insert(std::pair<Value*, ValueLifeInfo>(val, ValueLifeInfo(val)));
    }

    HdlState&   getInstructionState(Instruction* I);
    HdlCFGEdge& findCFGEdge(BasicBlock* src, BasicBlock* dst);
    CFGEdgeVector findAllCFGEdges(BasicBlock* src, BasicBlock* dst);
    void        addCFGEdge(HdlCFGEdge& edge);

    void dumpStateList();
    void dumpValueInfoMap();
};


}

