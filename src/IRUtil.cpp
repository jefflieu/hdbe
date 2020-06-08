#include "llvm/IR/CFG.h"

#include "IRUtil.hpp"


bool hdbe::isMemoryInstruction(Instruction* I)
{
  return (I->getOpcode() >= llvm::Instruction::MemoryOpsBegin &&  I->getOpcode() < llvm::Instruction::MemoryOpsEnd);
}

bool hdbe::isMemoryInstruction(Instruction &I) {
  return isMemoryInstruction(&I);
}

bool hdbe::isCtrlFlowInstruction(Instruction* I)
{
  return (I->getOpcode () == llvm::Instruction::Switch || I->getOpcode() == llvm::Instruction::Br);
}


bool hdbe::isPHIInstruction(Instruction* I) 
{
  return (I->getOpcode () == llvm::Instruction::PHI);
}

//Forware declaration 
#include "DataAnalyzer.hpp"
int hdbe::computeIndex(Instruction* I, Value* basePtr) {
  int index;
  Value * val = DataAnalyzer::analyzeMemoryOp(I, &index);
  assert(val == basePtr);
  return index;  
}

bool hdbe::isUselessInstruction(Instruction* I)
{
  if (I->getOpcode()==llvm::Instruction::Ret)
  {
    if (I->getNumOperands() > 0) 
      return llvm::Constant::classof(I->getOperand(0));
    else 
      return true;
  }

  return false;
}

/// Sometimes, the instructions operands are not really the input but rather the output for our own purposes
hdbe::ValuePtrVector hdbe::getInstructionInputs(Instruction* I, bool exclude_backedge)
{
  ValuePtrVector VPV;
  int opNum = I->getNumOperands();
  switch(I->getOpcode())
  {
    case llvm::Instruction::Br      : 
        assert(llvm::BranchInst::classof(I));
        if (static_cast<llvm::BranchInst*>(I)->isConditional())
          VPV.push_back(I->getOperand(0));
        break;
    case llvm::Instruction::Switch  : 
        assert(llvm::SwitchInst::classof(I));
        VPV.push_back(I->getOperand(0));
        for(int i = 2; i < opNum; i++)
        {
          if (i % 2 == 0)
          {
            LOG_IF_S(ERROR, ! llvm::Constant::classof(I->getOperand(i))) << "cases of switch must be constants\n";
            VPV.push_back(I->getOperand(i));
          }
        }
        break;
    case llvm::Instruction::PHI: 
      {
        llvm::PHINode* phi = static_cast<llvm::PHINode*>(I);
        for(int i = 0; i < phi->getNumIncomingValues(); i++)
        {
          llvm::BasicBlock* blk = phi->getIncomingBlock(i);
          llvm::Value* val = phi->getIncomingValue(i);
          //Simple loop 
          if (! isBackEdge(blk, phi->getParent()) || ! exclude_backedge)
          {
            VPV.push_back(static_cast<llvm::Value*>(blk));
            VPV.push_back(static_cast<llvm::Value*>(val));
          }
        }
        break;
      }
    default:
      for(const llvm::Use &use : I->operands())
      {
        llvm::Value* val = use.get();
        VPV.push_back(val);
      }
  }

  VPV.push_back(static_cast<Value*>(I->getParent()));

  return VPV;
}

//Kind of expensive routine !!!
bool hdbe::isBackEdge(BasicBlock *src, BasicBlock *dst)
{
  auto F = src->getParent();
  auto DT = llvm::DominatorTree(*F);
  auto LI = llvm::LoopInfo(DT);
  return (LI.isLoopHeader(dst) && (LI.getLoopFor(src) == LI.getLoopFor(dst)));
}

//These are Values that the Instruction produces
hdbe::ValuePtrVector hdbe::getInstructionOutputs(Instruction* I)
{
  ValuePtrVector VPV;
  int opNum = I->getNumOperands();
  VPV.push_back(static_cast<Value*>(I));
  switch(I->getOpcode())
  {
    case llvm::Instruction::Br      : 
        assert(llvm::BranchInst::classof(I));
        if (static_cast<llvm::BranchInst*>(I)->isConditional()) {
          if (! isBackEdge(static_cast<llvm::BranchInst*>(I)->getParent(), static_cast<llvm::BasicBlock*>(I->getOperand(1))))
            VPV.push_back(I->getOperand(1));
          if (! isBackEdge(static_cast<llvm::BranchInst*>(I)->getParent(), static_cast<llvm::BasicBlock*>(I->getOperand(2))))
            VPV.push_back(I->getOperand(2));
        } else 
          VPV.push_back(I->getOperand(0));
        break;
    case llvm::Instruction::Switch  : 
        assert(llvm::SwitchInst::classof(I));
        VPV.push_back(I->getOperand(1));
        for(int i = 2; i < opNum; i++)
        {
          if (i % 2 == 1)
          {
            LOG_IF_S(ERROR, ! llvm::BasicBlock::classof(I->getOperand(i))) << "target of switch must be BasicBlock\n";
            assert(llvm::BasicBlock::classof(I->getOperand(i)));
            if (! isBackEdge(static_cast<llvm::BranchInst*>(I)->getParent(), static_cast<llvm::BasicBlock*>(I->getOperand(i))))
              VPV.push_back(I->getOperand(i));
          }
        }
        break;
    default:
      break;
  }

  return VPV;
}

String hdbe::getBriefInfo(Value* v)
{
  char buf[256];
  int size;
  String s;
  if(Instruction::classof(v)) {
    auto I = static_cast<Instruction*>(v);
    size = sprintf(buf, "%15s: %20s (%lx)","Instruction", I->getOpcodeName(), reinterpret_cast<uint64_t>(v));
    s = String(buf, size);
  } else if(BasicBlock::classof(v)){
    size = sprintf(buf, "%15s: %20s (%lx)","BasicBlock", v->getName().data(), reinterpret_cast<uint64_t>(v));
    s = String(buf, size);
  } else { 
    size = sprintf(buf, "%15s: %20s (%lx)","Value", v->getName().data(), reinterpret_cast<uint64_t>(v));
    s = String(buf, size);
  }
  return s;
}
