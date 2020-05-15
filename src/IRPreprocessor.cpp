#include <list>
#include <string>
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"



#include "IRPreprocessor.hpp"
#include "HDLUtil.hpp"

#ifndef IR_PP_DBG 
#define IR_PP_DBG 5
#endif 

using namespace hdbe;


void IRPreprocessor::transformNames()
{
  for (Module::global_iterator I = irModule->global_begin(), E = irModule->global_end(); I != E; ++I)
  {
    String newName = makeHdlName(I->getName().str());
    I->setName(newName);
    LOG_S(IR_PP_DBG) << I->getName() << "\n";
  }
  for (Module::iterator F = irModule->begin(), F_end = irModule->end(); F != F_end; ++F)
  {
    for(Function::iterator B = F->begin(), B_end = F->end(); B != B_end; ++B)
    {
      B->setName(makeHdlName(B->getName().str()));
    }
    for (llvm::inst_iterator I = inst_begin(&*F), E = inst_end(&*F); I != E; ++I)
    {
      //Transform name of use 
      if (I->getType()->isVoidTy()) continue;
      I->setName(makeHdlName(I->getName().str()));
      LOG_S(IR_PP_DBG) << I->getName() << "\n";
        
    }
  }
}

void IRPreprocessor::balanceCFG()
{
  //Identify the triangles: 
  LOG_START(INFO);
  std::pair<BasicBlock*, BasicBlock*> edge;
  std::map<hash_code , std::pair<BasicBlock*, BasicBlock*>> edgeMap;
  for(BasicBlock & bb : irFunction->getBasicBlockList())
  {
    _log_stdout << bb.getName() << "\n";
    for(BasicBlock* succ: successors(&bb))
    {
      for(BasicBlock* succ_succ: successors(succ))
      {
        for(BasicBlock* pred_succ_succ: predecessors(succ_succ))
        {
          if (pred_succ_succ  == &bb)
          {
            edge = std::make_pair(&bb, succ_succ);
            hash_code hc = hash_value(edge);
            edgeMap[hc] = edge;
          }
        }
      }  
    }
  }

  for(auto item: edgeMap)
  {
    //_log_stdout << "Hash : " << item.first << " : " << item.second.first->getName() << " : " << item.second.second->getName() << "\n";
    BasicBlock * src = item.second.first;
    BasicBlock * dst = item.second.second;
    Twine name = Twine(src->getName() + dst->getName());

    Instruction& firstInst = dst->front();  
    //BasicBlock* newBB = lastBB.splitBasicBlock(&firstInst);
    //_log_stdout << "last Block " << lastBB;
    //_log_stdout << "New  Block " << *newBB;
    BasicBlock* newBB = llvm::BasicBlock::Create(dst->getContext(), name,
                              dst->getParent(),
                              dst);
    Instruction* brInst = llvm::BranchInst::Create(dst, newBB);
    Instruction* srcTerm = src->getTerminator();
    srcTerm->replaceSuccessorWith(dst, newBB);
    dst->replacePhiUsesWith(src, newBB);
  }
  _log_stdout<<"BalancedCFG\n";
  _log_stdout << *irFunction;
}