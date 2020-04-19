#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "DataAnalyzer.hpp"

#define  DA_DBG 2

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
  auto &memOpsList    = CDI_h->memOpsList; 
  
  //Iterate over argument 
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(); I != E; ++I)
  {
    LOG_S(DA_DBG) << "Argument " << *I << "\n";     
    portList.push_back(HdlPort((llvm::Value*)&*I));                                       
    HdlPort &port = portList.back();
    port.property       = analyzeValue(&*I);    
  }

  for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    //Filter out memory related operations 
    if ( I->getOpcode() >= llvm::Instruction::MemoryOpsBegin &&  I->getOpcode() <= llvm::Instruction::MemoryOpsEnd) 
     {
       memOpsList.push_back(HdlVariable((llvm::Value*)&*I));
       continue;
     }
    for(llvm::Use &use : I->operands()) {
      llvm::Value* val = use.get();      
      if (! isIn(variableList, val) && ! isIn(portList, val) && ! isIn(memOpsList, val)) {
        variableList.push_back(HdlVariable((llvm::Value*)val));
        HdlVariable &var = variableList.back();
        var.property = analyzeValue(val);
        LOG_S(DA_DBG) << val << " : " << *val << "\n";        
      } else 
        LOG_S(DA_DBG) << val << " : already in the list\n";            
    }
  }    
   
}

HdlProperty DataAnalyzer::analyzeValue(llvm::Value* value)
{ 
  HdlProperty property;
  switch(value->getType()->getTypeID()) 
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
  
  return property;
}

HdlProperty DataAnalyzer::analyzePointer(llvm::Value* valuePointerTy)
{
  LOG_S(DA_DBG) << "Pointer analysis: " << *valuePointerTy << " ( ID = " << valuePointerTy << ")\n";
  bool staticIndex = true;
  int maxIdx = -1;
  const DataLayout & DL = CDI_h->irModule->getDataLayout();

  HdlProperty property;
  assert(valuePointerTy->getType()->getPointerElementType()->getTypeID() == llvm::Type::IntegerTyID);
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
                                                for(llvm::Use &use : U->operands())
                                                {
                                                  llvm::Value* val = use.get();                                      
                                                  if (val == valuePointerTy) continue; 
                                                  if (!(D_IS_CONST_OPERAND(val))) {
                                                      staticIndex = false; 
                                                      break;
                                                      }
                                                  else{ 
                                                      assert(llvm::ConstantInt::classof(val));
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
  property.isConstant = ConstantInt::classof(valuePointerTy);  
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

  LOG_S(DA_DBG) << " end analysis \n";
  return property;
} 



