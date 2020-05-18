#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "DataAnalyzer.hpp"
#include "IRUtil.hpp"

#ifndef  DA_DBG 
#define  DA_DBG 0
#endif 

using namespace hdbe;


void DataAnalyzer::analyze(Module * irModule, Function * irFunction)
{
  Function* F = irFunction;
  auto &portList      = CDI_h->portList; 
  auto &variableList  = CDI_h->variableList; 
  auto &memObjList    = CDI_h->memObjList; 
  
  LOG_START(INFO);

  //Iterate over argument 
  //For each argument, we generate 2 HdlObjects: 
  //1 HdlPort and and 1 HdlVariable. 
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(); I != E; ++I)
  {
    LOG_S(DA_DBG + 1) << "Argument " << *I << "\n";     
    portList.push_back(HdlPort((llvm::Value*)&*I));                                       
    HdlPort &port       = portList.back();
    port.property       = analyzeValue(&*I);
    variableList.push_back(HdlVariable((llvm::Value*)&*I));
    HdlVariable &var    = variableList.back();
    var.property        = port.property;
    var.property.stype  = HdlSignalType::regType;   
  }

  //Function is a special output port 
  portList.push_back(HdlPort(static_cast<llvm::Value*>(F)));                                       
  HdlPort &output = portList.back();
  output.property = analyzeValue(static_cast<llvm::Value*>(F));
  output.name     = "func_ret";  

  for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    //Filter out memory related operations 
    if (isMemoryInstruction(&*I)) 
    {
      //Find the base pointer of the memory instruction and generate and Hdl Object associated with the base Ptr
      int index; //Not used
      Value* val = analyzeMemoryOp(&*I, &index);
      
      if ( ! isIn(memObjList, val) ) {
        memObjList.push_back(HdlMemory((llvm::Value*)val));
        HdlMemory   &memObj = memObjList.back();
        memObj.property     = analyzePointer(val);
        memObj.memInstrList.push_back(&*I);
        LOG_S(DA_DBG+1) << val << " : " << *val << "\n";    
      } else {
        LOG_S(DA_DBG+1) << val << " : " << *val << " already in the list\n";
        HdlMemory  &memObj = find(memObjList, val); 
        memObj.memInstrList.push_back(&*I);
      }

    }
    //
    ValuePtrVector depList = getInstructionInputs(&*I);
    //for(llvm::Use &use : I->operands()) {
    // llvm::Value* val = use.get();     
    for(Value* val : depList) {
      if (! isIn(variableList, val) && ! isIn(portList, val) && ! isIn(memObjList, val)) {
        variableList.push_back(HdlVariable((llvm::Value*)val));
        HdlVariable &var = variableList.back();
        var.property = analyzeValue(val);
        LOG_S(DA_DBG + 1) << val << " : " << *val << "\n";        
      } else 
        LOG_S(DA_DBG + 1) << val << " : " << *val << " already in the list\n";      
    }
  }

  /*LOG(DA_DBG, "memory operations");
  for(auto I = memObjList.begin(), E = memObjList.end(); I!=E; ++I)
  {
    LOG_S(DA_DBG + 1) << *(I->getIrValue()) << "\n";
  }*/
  
  LOG_DONE(INFO);
}

HdlProperty DataAnalyzer::analyzeValue(llvm::Value* value)
{ 
  HdlProperty property;
  llvm::Type *type_h = value->getType();
  LOG_S(DA_DBG + 1) << "Value analysis \n";
  LOG_S(DA_DBG + 1) << *value << "(" << value << ")\n";
  //Special handling of a function 
  if (Function::classof(value)) {
    type_h = static_cast<Function*>(value)->getReturnType();
    property.stype = HdlSignalType::outputType;
    if (type_h->getTypeID() == llvm::Type::IntegerTyID)
      property.bitwidth = type_h->getIntegerBitWidth();  
    else 
      property.bitwidth  = 1;
    return property;
  }

  //Special handling of a basicblock 
  if (BasicBlock::classof(value)) {
    property.stype = HdlSignalType::regType;
    property.bitwidth  = 1;
    return property;
  }  

  switch(type_h->getTypeID()) 
    {
      case llvm::Type::IntegerTyID :  { 
                                        property.vtype = HdlVectorType::scalarType;
                                        /* 
                                          Decide of the value is a port or not 
                                          We currently only support input
                                        */
                                        if (Argument::classof(value))
                                          property.stype = HdlSignalType::inputType;                                   
                                        else 
                                          property.stype = HdlSignalType::regType;
                                        
                                        property.bitwidth = value->getType()->getIntegerBitWidth();      
                                        property.isConstant = ConstantInt::classof(value);
                                        if (property.isConstant)
                                          property.dflt = (static_cast<llvm::ConstantInt*>(value))->getSExtValue();
                                        break; 
                                      }
      case llvm::Type::PointerTyID :  {
                                        property = analyzePointer(value);                                               
                                        break;
                                      }
      default :   
        LOG_ERROR_S << "Not supported type " << "\n";
        LOG_ERROR_S << *value << "\n";
        LOG_ERROR_S << "Type ID " << type_h->getTypeID() << "\n"; 
        break;
    }
  property.isUnused = value->users().empty();
  property.isBackValue = isBackValue(value);
  LOG_S(DA_DBG + 2) << " isConstant  : " << property.isConstant << "\n";
  LOG_S(DA_DBG + 2) << " bitwidth    : " << property.bitwidth << "\n";
  LOG_S(DA_DBG + 2) << " arraylength : " << property.arraylength << "\n";
  LOG_S(DA_DBG + 2) << " isUnused    : " << property.isUnused << "\n";
  LOG_S(DA_DBG + 2) << " isBackValue : " << property.isBackValue << "\n";
  LOG_S(DA_DBG + 1) << " end analysis \n";
  return property;
}

HdlProperty DataAnalyzer::analyzePointer(llvm::Value* valuePointerTy)
{
  LOG_S(DA_DBG+1) << "Pointer analysis: \n"; 
  LOG_S(DA_DBG+1) << *valuePointerTy << " ( ID = " << valuePointerTy << ")\n";
  bool staticIndex = true;
  bool readOnly  = true;
  bool writeOnly = true;
  int maxIdx = 0;
  const DataLayout & DL = CDI_h->irModule->getDataLayout();

  HdlProperty property;
  ASSERT(valuePointerTy->getType()->getPointerElementType()->getTypeID() == llvm::Type::IntegerTyID, 
    "Not supported pointer type\n");
  for(llvm::User * U : valuePointerTy->users())
  {
    LOG_S(DA_DBG) << *U << " valueID: " << U->getValueID() << "\n";    
    if (llvm::Instruction::classof(U)){            
      llvm::Instruction* I = (llvm::Instruction*)(U);
      switch(I->getOpcode())
      {
        case llvm::Instruction::GetElementPtr : /*  
                                                    Check the Get element is static, then we may promote it to registers 
                                                 */
                                                LOG_S(DA_DBG + 2) << "TypeID: " << I->getType()->getTypeID() << "\n";
                                                for(llvm::Use &use : U->operands())
                                                {
                                                  llvm::Value* val = use.get();                                      
                                                  if (val == valuePointerTy) continue; 
                                                  if (!llvm::Constant::classof(val)) {
                                                      staticIndex = false; 
                                                      break;
                                                      }
                                                  else{ 
                                                      ASSERT(llvm::ConstantInt::classof(val), " only support simple indexing\n");
                                                      //outs() << "Value of operand " << ((llvm::ConstantInt*)val)->getZExtValue() << "\n";
                                                      maxIdx = std::max(maxIdx, (int)((llvm::ConstantInt*)val)->getZExtValue());
                                                    }
                                                          
                                                }
                                                break;
        case llvm::Instruction::Load          : writeOnly = false; break;
        case llvm::Instruction::Store         : readOnly  = false; break;
        default : staticIndex = false; LOG(ERROR,  I->getOpcodeName() << " is not supported \n");
      }
    }
  }
  
  if (Argument::classof(valuePointerTy)) {
    if (readOnly && !writeOnly)
      property.stype = HdlSignalType::inputType;
    else if (!readOnly and writeOnly)
      property.stype = HdlSignalType::outputType;
    else if (!readOnly && !writeOnly)
      property.stype = HdlSignalType::inoutType;
  } else 
    property.stype = HdlSignalType::regType;
  
  property.isConstant = false; //llvm::Constant::classof(valuePointerTy);  
  
  property.vtype = (maxIdx < 0)? HdlVectorType::scalarType : (staticIndex? HdlVectorType::arrayType : HdlVectorType::memoryType);
  
  if (property.vtype == HdlVectorType::scalarType) {
    property.bitwidth = valuePointerTy->getType()->getPointerElementType()->getIntegerBitWidth();
    property.arraylength = 0;
  } else if (property.vtype == HdlVectorType::arrayType) {
    property.bitwidth = valuePointerTy->getType()->getPointerElementType()->getIntegerBitWidth();
    property.arraylength = maxIdx + 1;
  } else {
    property.bitwidth    = valuePointerTy->getType()->getPointerElementType()->getIntegerBitWidth();
    property.arraylength = DL.getPointerSizeInBits();  
  }
  property.isUnused = valuePointerTy->users().empty();

  LOG_S(DA_DBG + 2) << " isConstant  : " << property.isConstant << "\n";
  LOG_S(DA_DBG + 2) << " bitwidth    : " << property.bitwidth << "\n";
  LOG_S(DA_DBG + 2) << " arraylength : " << property.arraylength << "\n";
  LOG_S(DA_DBG + 2) << " ReadOnly    : " << readOnly << "\n";
  LOG_S(DA_DBG + 2) << " WriteOnly   : " << writeOnly << "\n";
  LOG_S(DA_DBG + 2) << " isUnused    : " << property.isUnused << "\n";
  LOG_S(DA_DBG + 2) << " isUnused    : " << property.isBackValue << "\n";
  LOG_S(DA_DBG + 1) << " end analysis \n";
  return property;
} 


Value* DataAnalyzer::analyzeMemoryOp(Instruction * memOp, int* index)
{
  bool staticIndex = false;
  Value * basePtr = nullptr;
  Value * idxPtr  = nullptr;
  LOG_S(DA_DBG + 1) << "Analyzing memory ops" << *memOp << "\n";
  switch(memOp->getOpcode())
      {
        case llvm::Instruction::GetElementPtr : basePtr = memOp->getOperand(0);
                                                idxPtr  = memOp->getOperand(1);
                                                LOG_S(DA_DBG + 1) << "TypeID   : " << memOp->getType()->getTypeID() << "\n";
                                                LOG_S(DA_DBG + 1) << "Base Ptr : " << *basePtr << " (" << basePtr << ")\n";
                                                LOG_S(DA_DBG + 1) << "Indx Ptr : " << *idxPtr << " (" << idxPtr << ")\n";
                                                if (llvm::ConstantInt::classof(idxPtr))
                                                  *index = (int)((llvm::ConstantInt*)idxPtr)->getZExtValue();
                                                else 
                                                  *index = -1;
                                                break;

        case llvm::Instruction::Load          : basePtr = static_cast<llvm::LoadInst*>(memOp)->getPointerOperand(); 
                                                LOG_S(DA_DBG + 1) << "Load Ptr : " << *basePtr << " (" << basePtr << ")\n";
                                                *index = 0;
                                                break;
                                                
        case llvm::Instruction::Store         : basePtr = static_cast<llvm::StoreInst*>(memOp)->getPointerOperand(); 
                                                LOG_S(DA_DBG + 1) << "Store Ptr : " << *basePtr << " (" << basePtr << ")\n";
                                                *index = 0;
                                                break;
                                                
        default : staticIndex = false; LOG_S(ERROR) << memOp->getOpcodeName() << " is not supported \n";
      }
  if (Instruction::classof(basePtr)) 
    basePtr = analyzeMemoryOp(static_cast<Instruction*>(basePtr), index);
   LOG_S(DA_DBG + 1) << "done\n";
  return basePtr;
}



void DataAnalyzer::analyzeBasicBlocks(Module* irModule, Function* irFunction)
{
  //Walk the Basicblocks 
  /*
  llvm::AnalysisManager<Function> FAM;
  llvm::LoopAnalysis LA;
  llvm::LoopInfo LI = LA.run(*irFunction, FAM);
  llvm::BranchProbabilityAnalysis BPA;
  BPA.run(*irFunction, FAM);
  //llvm::BranchProbabilityInfo BPI(*irFunction, LI);
  //llvm::BlockFrequencyInfo BFI(*irFunction, , LI);
  */
  auto &TL = CDI_h->transitionList;

  LOG_START(INFO);
  
  //Some pre-check 
  for(BasicBlock & bb : irFunction->getBasicBlockList())
  {
    if (pred_size(&bb) > 1)
      for(BasicBlock* pred : predecessors(&bb))
      {
        Instruction* term = pred->getTerminator();
        LOG_IF_S(FATAL, !term) << "Basic block is not well-formed\n";
        assert(term);
      }
    assert(pred_size(&bb) <= 16);
  }

  std::map<BasicBlock*, float> blockFrequency;
  for(BasicBlock & bb : irFunction->getBasicBlockList())
  {
    LOG_S(DA_DBG + 1) << bb.getName() << "\n";
    blockFrequency[&bb] = 0.0;
  }
  BasicBlock &entry = irFunction->getEntryBlock();
  blockFrequency[&entry] = 1.0;

  for(BasicBlock & bb : irFunction->getBasicBlockList())
  {
    LOG_S(DA_DBG + 1) << bb.getName() << " has: " << succ_size(&bb) << "successors \n";
    int succNum = succ_size(&bb);
    //for(BasicBlock* succ : successors(&bb))
    for(auto succ = succ_begin(&bb); succ != succ_end(&bb); ++succ)
    {
       LOG_S(DA_DBG + 2) << " => " << succ->getName() << "\n"; 
       blockFrequency[*succ]+=blockFrequency[&bb]/succNum;

       //Generating edges
       TL.push_back(HdlCFGEdge(bb.getTerminator(), succ.getSuccessorIndex()));
       HdlCFGEdge &edge = TL.back();
       edge.property.bitwidth = 1;
       edge.property.stype    = HdlSignalType::regType;
       LOG_S(DA_DBG + 1) << "Edge: " << edge.getSrcBB()->getName() << " -> " << edge.getDestBB()->getName() << "\n";
    }
  }

  for(auto item : blockFrequency)
  {
    LOG_S(DA_DBG + 1) << item.first->getName() << ":" << item.second << "\n";
  }

  //for(auto &bb = llvm::depth_first(&(irFunction->getEntryBlock())))
  _log_stdout << "DepthFirstIterator\n";
  using IterType = llvm::df_iterator<BasicBlock*>;
  for(IterType df_iter = IterType::begin(&(irFunction->getEntryBlock())); 
        df_iter != IterType::end(&(irFunction->getEntryBlock())); 
          ++df_iter)
  {
    _log_stdout << df_iter->getName().str() << "\n";
  }  
  
  LOG_DONE(INFO);
}



bool DataAnalyzer::isBackValue(Value* v)
{
  //A value is a backvalue if it is used by a PHI node 
  //And the incoming edge is a back-edge 
  bool is_fed_back = false;
  if (!llvm::Instruction::classof(v)) return is_fed_back;
  for (User* user : v->users())
  {
    if (llvm::PHINode::classof(user))
    {
      auto phi = static_cast<llvm::PHINode*>(user);
      for(int i = 0; i<phi->getNumIncomingValues(); i++)
        { 
          BasicBlock* bb = phi->getIncomingBlock(i);
          Value* value = phi->getIncomingValue(i);          
          if (value == v && bb == (static_cast<llvm::Instruction*>(v))->getParent()) is_fed_back = true;
        }
    }
  }
  return is_fed_back;
}





