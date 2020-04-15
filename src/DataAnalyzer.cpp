#include "DataAnalyzer.hpp"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

using namespace hdbe;
using Function = llvm::Function;
using GlobalValue = llvm::GlobalValue;
using DataLayout = llvm::DataLayout;
using Module = llvm::Module;
using llvm::outs;

void DataAnalyzer::analyze(std::string funcName) 
{
  Function* F = m_irModule->getFunction(funcName);
  //Iterate over argument 
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(); I != E; ++I)
  {
    outs() << *I << "\n"; 
    outs() << "Argument " << I->getName() << " type " << I->getType() << " \n" ; 
    m_portList.push_back(HdlPort((llvm::Value*)&*I));                                       
    HdlPort &port = m_portList.back();
    port.m_property       = analyzeValue(&*I);
    port.m_property.stype = HdlSignalType::inputType;
    //auto type = I->getType(); 
    //outs() << " is integer: " << type->isIntegerTy() << " pointer: " << type->isPointerTy() << " structure " << type->isStructTy() << " array " << type->isArrayTy() << "\n";
    
  }

  for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    
    for(llvm::Use &use : I->operands()) {
      llvm::Value* val = use.get();      
      if (! isIn(m_variableList, val) && ! isIn(m_portList, val)) {
        m_variableList.push_back(HdlVariable((llvm::Value*)val));
        HdlVariable &var = m_variableList.back();
        var.m_property = analyzeValue(val);
        var.m_property.stype = HdlSignalType::regType;
        LOG_S(6) << "Collecting " << val << "(" << val->getName().str() << ")";
        val->dump();
      } else 
        LOG_S(6) << "Already in " << val;      
      
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
                                        property.stype = HdlSignalType::inputType;
                                        property.bitwidth = value->getType()->getIntegerBitWidth();                                        
                                        break; 
                                      }
      case llvm::Type::PointerTyID :  {
                                        property = analyzePointer(value);                                        
                                        break;
                                      }
      default :   
        LOG_S(0) << "Not supported type at the port" << "\n";
        break;
    }
  
  return property;
}

HdlProperty DataAnalyzer::analyzePointer(llvm::Value* valuePointerTy)
{
  LOG_S(4) << "Pointer analysis \n";
  bool staticIndex = true;
  int maxIdx = -1;
  const DataLayout & DL = m_irModule->getDataLayout();

  HdlProperty property;
  assert(valuePointerTy->getType()->getPointerElementType()->getTypeID() == llvm::Type::IntegerTyID);
  for(llvm::User * U : valuePointerTy->users())
  {
    //outs() << *U << " valueID: " << U->getValueID() << "\n";    
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
        default : staticIndex = false; outs() << I->getOpcodeName() << " is not supported \n";
      }
    }
  }
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

  //outs() << "Indexing is static " << staticIndex << "\n";
  LOG_S(4) << "--- end analysis \n";
  return property;
} 



