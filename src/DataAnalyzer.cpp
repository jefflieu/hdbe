#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "DataAnalyzer.hpp"
#include "IRUtil.hpp"

#define  DA_DBG 1

using namespace hdbe;
using Function    = llvm::Function;
using GlobalValue = llvm::GlobalValue;
using DataLayout  = llvm::DataLayout;
using Module      = llvm::Module;
using Argument    = llvm::Argument;
using ConstantInt = llvm::ConstantInt;



void DataAnalyzer::analyze(Module * irModule, Function * irFunction)
{
  Function* F = irFunction;
  auto &portList      = CDI_h->portList; 
  auto &variableList  = CDI_h->variableList; 
  auto &memObjList    = CDI_h->memObjList; 
  
  LOG_S(DA_DBG) << " analyzing \n";     
    
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
  portList.push_back(HdlPort(reinterpret_cast<llvm::Value*>(F)));                                       
  HdlPort &output = portList.back();
  output.property = analyzeValue(reinterpret_cast<llvm::Value*>(F));
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
    else

      for(llvm::Use &use : I->operands()) {
        llvm::Value* val = use.get();      
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
        LOG_S(ERROR) << "Not supported type at the port" << "\n";
        break;
    }
  LOG_S(DA_DBG + 2) << " isConstant  : " << property.isConstant << "\n";
  LOG_S(DA_DBG + 2) << " bitwidth    : " << property.bitwidth << "\n";
  LOG_S(DA_DBG + 2) << " arraylength : " << property.arraylength << "\n";
  LOG_S(DA_DBG + 1) << " end analysis \n";
  return property;
}

HdlProperty DataAnalyzer::analyzePointer(llvm::Value* valuePointerTy)
{
  LOG_S(DA_DBG+1) << "Pointer analysis: \n"; 
  LOG_S(DA_DBG+1) << *valuePointerTy << " ( ID = " << valuePointerTy << ")\n";
  bool staticIndex = true;
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
        case llvm::Instruction::Load          : break;
        case llvm::Instruction::Store         : break;
        default : staticIndex = false; LOG_S(ERROR) << I->getOpcodeName() << " is not supported \n";
      }
    }
  }
  
  if (Argument::classof(valuePointerTy))
    property.stype = HdlSignalType::inputType;
  else 
    property.stype = HdlSignalType::regType;
  
  property.isConstant = llvm::Constant::classof(valuePointerTy);  
  
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
  
  LOG_S(DA_DBG + 2) << " isConstant  : " << property.isConstant << "\n";
  LOG_S(DA_DBG + 2) << " bitwidth    : " << property.bitwidth << "\n";
  LOG_S(DA_DBG + 2) << " arraylength : " << property.arraylength << "\n";
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



