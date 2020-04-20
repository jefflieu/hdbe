#include "llvm/ADT/Twine.h"
#include "llvm/IR/InstrTypes.h"

#include "ValueLifeInfo.hpp"
#include "logging/logger.hpp"
#include "InstructionScheduler.hpp"

#define  IS_DBG 6

using namespace std;
using namespace hdbe;
using Twine       = llvm::Twine;
using Value       = llvm::Value;
using Instruction = llvm::Instruction;
using Function    = llvm::Function;
using Module      = llvm::Module;

void InstructionScheduler::schedule(BasicBlock * irBasicBlock){};

void InstructionScheduler::schedule() {
 this->schedule(CDI_h->irFunction);
}

void InstructionScheduler::schedule(Function * irFunction) 
{
  auto &M              = *(CDI_h->irModule);
  auto &F              = *irFunction;
  //auto &HWD            = CDI_h->HWD;
  auto &stateList      = CDI_h->stateList;
  auto &VIM            = CDI_h->valueInfoMap;
  auto &entryBlock     = *(F.begin());
  
  LOG(INFO, "Scheduling of function " << F.getName());  
  
  // Map instructions to SPACE and TIME        
        
    
  LOG_S(IS_DBG) << "Assigning birth time to global variables and arguments \n";      
    
  //Assign birth time for Global Variable 
  for(auto gv_i = M.global_begin(), gv_last = M.global_end(); gv_i != gv_last; ++gv_i)
    {
      LOG_S(IS_DBG) << *gv_i << "\n" ;
      auto ret = CDI_h->addValueInfo(&*gv_i);
      ret.first->second.setBirthTime(&entryBlock, 0.0);
    }

    //Assign valid_time for Arguments:   
  for(auto arg_i = F.arg_begin(), arg_last = F.arg_end(); arg_i != arg_last; arg_i ++ )
    {
      LOG_S(IS_DBG) << *arg_i << "\n";
      auto ret = CDI_h->addValueInfo(&*arg_i);
      ret.first->second.setBirthTime(&entryBlock, 0.0);     
    }

  LOG(INFO, "... done ");
}

