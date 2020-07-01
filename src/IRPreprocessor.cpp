/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include <list>
#include <string>
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"


#include "IRPreprocessor.hpp"
#include "HDLUtil.hpp"

#ifndef IR_PP_DBG 
#define IR_PP_DBG 9
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

void IRPreprocessor::processConstantExpr()
{
  for (Module::iterator F = irModule->begin(), F_end = irModule->end(); F != F_end; ++F)
  {
    for (llvm::inst_iterator I = inst_begin(&*F), E = inst_end(&*F); I != E; ++I)
    {
      processConstantExpr(&*I);
    }
  }
}

bool IRPreprocessor::processConstantExpr(Instruction* I)
{
  for(Value *value : I->operands())
  {
    ConstantExpr * ce = nullptr;
    if (ce = llvm::dyn_cast<ConstantExpr>(value))
    {
      Instruction *ce_instr = ce->getAsInstruction();
      if(ce_instr->getOpcode() == Instruction::GetElementPtr)
      {
        String name = "constExpr_" + getValueHdlName(static_cast<Value*>(ce_instr));
        ce_instr->setName(makeHdlName(name));
        ce_instr->insertBefore(I);
        static_cast<User*>(I)->replaceUsesOfWith(value, static_cast<Value*>(ce_instr));
      }
      LOG_S(0) << "Found constant expression " << *ce;
    }
  }
  return true;
} 

///This function is deprecated. 
///This is used to insert an empty BasicBlock to the edge that jumps BasicBlock level
void IRPreprocessor::balanceCFG()
{
  //Identify the triangles: 
  LOG_START(INFO);
  std::pair<BasicBlock*, BasicBlock*> edge;
  std::map<hash_code , std::pair<BasicBlock*, BasicBlock*>> edgeMap;
  for(BasicBlock & bb : irFunction->getBasicBlockList())
  {
    LOG_S(IR_PP_DBG + 1) << bb.getName() << "\n";
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
  LOG_S(IR_PP_DBG + 1)<<"BalancedCFG\n";
  LOG_S(IR_PP_DBG + 1) << *irFunction;
  LOG_DONE(INFO);

}

/*
  The memory model of clang is not really compatible with memory model of HDL 
  Clang sometimes muxes multiple store instructions using PHI nodes which causes 
  the backend troubles. This function aims to reverse that optimization (Code sinking) done by clang
  It detects the pattern and clones it, then insert the cloned block of instructions into the predecessors 
  and removes the sunk instructions
*/
void IRPreprocessor::removePointerPHI()
{
  InstPointerVector worklist;
  InstPointerVector clonelist;
  std::map<Instruction*, Instruction*> clonemap;
  std::map<Instruction*, Instruction*> store_phi_map;
  Instruction* store_inst = nullptr;
  Instruction* phi_inst = nullptr;

  //First pass find out PHI nodes that produce pointers
  //We only support PHI-Pointer to Store instructions
  for (Module::iterator F = irModule->begin(), F_end = irModule->end(); F != F_end; ++F)
  {
    for (llvm::inst_iterator I = inst_begin(&*F), E = inst_end(&*F); I != E; ++I)
    {
      if (llvm::PHINode::classof(&(*I)) && I->getType()->isPointerTy() && I->getName().endswith(StringRef(".sink"))) 
      {
        phi_inst = static_cast<llvm::Instruction*>(&(*I));
        LOG_S(INFO) << *I << " is PHINode with SINK tag\n";
        
        unsigned user_cnt = 0;
        for(User* user : I->users())
        {
          assert(llvm::StoreInst::classof(user) && "The user of PointerPHI must be Store instruction");
          store_inst = static_cast<llvm::Instruction*>(user);
          assert(store_inst->getParent() == I->getParent() && "The user of PointerPHI must be in the same basicblock");
          user_cnt ++;
        }
        assert(user_cnt == 1 && "The user of the PointerPHI must be the only one\n");
        assert(store_inst);
        phi_inst = &(*I);
        store_phi_map[store_inst] = phi_inst;
      }
    }
  }

  for(auto item : store_phi_map)
  {
    
    //Construct the worklist which is a list of instructions related to the "Store" instruction
    store_inst = item.first;
    phi_inst = item.second;
    worklist.push_back(store_inst);
    getRelatedValues(store_inst, store_inst->getParent(), worklist);
    
    //Then iterate over the incoming blocks
    for(unsigned n = 0; n < static_cast<llvm::PHINode*>(phi_inst)->getNumIncomingValues(); n++){
      
      //For each incoming block, clone the worklist
      LOG_S(INFO) << "Cloning " << n << "\n";
      clonelist.clear();
      for(auto i : worklist)
      {
        LOG_S(INFO) << *i << "\n";
        Instruction *clone = i->clone();

        //Set name of cloned instruction if necessary 
        if (! i->getName().empty())
          clone->setName(i->getName() + Twine(".") + static_cast<llvm::PHINode*>(phi_inst)->getIncomingBlock(n)->getName());

        clonelist.push_back(clone);
        clonemap[i] = clone;
      }  
      
      //Replacing references
      for(auto i : clonelist)
      {
        for (Value * value : i->operands())
        {
          if (clonemap.count(static_cast<Instruction*>(value))==1)
          {
            static_cast<User*>(i)->replaceUsesOfWith(value, clonemap[static_cast<Instruction*>(value)]);
          } else if (llvm::PHINode::classof(value)) {
            static_cast<User*>(i)->replaceUsesOfWith(value, static_cast<llvm::PHINode*>(value)->getIncomingValue(n));
          } else {
            assert(! "Weird error");
          }
        }
        LOG_S(INFO) << *i << "\n";
      }

      //Now inserting the instruction to the block 
      Instruction* blockTerminator =   static_cast<llvm::PHINode*>(phi_inst)->getIncomingBlock(n)->getTerminator();
      assert(blockTerminator && "Block terminator is not unique or not existed");
      for(auto i : clonelist)
      {
        i->insertBefore(blockTerminator);
      }
    }

    //Remove the original instructions after cloning
    for(auto i : worklist)
    {
      i->eraseFromParent();
    }
    phi_inst->eraseFromParent();
  }

}

void IRPreprocessor::getRelatedValues(Instruction* I, BasicBlock* bb, InstPointerVector & ipv)
{
  Instruction* inst = nullptr;
  for(Value *operand : I->operands())
  {
    if (inst = llvm::dyn_cast<Instruction>(operand))
    {
      if (inst->getParent() != bb || llvm::PHINode::classof(operand)) continue;
      ipv.push_back(inst);
      getRelatedValues(inst, bb, ipv);
    }
  }
}





