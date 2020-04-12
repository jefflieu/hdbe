#include "DataAnalyzer.hpp"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

using namespace hdbe;
using Function = llvm::Function;
using DataLayout = llvm::DataLayout;
using llvm::outs;

void DataAnalyzer::analyze(std::string funcName) 
{
  Function* F = m_irModule->getFunction(funcName);
  //Iterate over argument 
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(); I != E; ++I)
  {
    outs() << *I << "\n"; 
    outs() << "Argument " << I->getName() << " type " << I->getType() << " \n" ; 
    auto type = I->getType(); 
    outs() << " is integer: " << type->isIntegerTy() << " pointer: " << type->isPointerTy() << " structure " << type->isStructTy() << " array " << type->isArrayTy() << "\n";
    switch(type->getTypeID()) 
    {
      case llvm::Type::IntegerTyID :  {
                                        m_portList.push_back(HdlPort((llvm::Value*)&*I));                                       
                                        HdlPort &port = m_portList.back();
                                        port.m_property.vtype = HdlVectorType::scalarType;
                                        port.m_property.stype = HdlSignalType::portType;
                                        port.m_property.bitwidth = type->getIntegerBitWidth();                                        
                                        break; 
                                      }
      case llvm::Type::PointerTyID :  {
                                        m_portList.push_back(HdlPort((llvm::Value*)&*I));
                                        HdlPort &port = m_portList.back();
                                        port.m_property = analyzePointer(&*I);
                                        port.m_property.stype = HdlSignalType::portType;
                                        
                                                                                  
                                        break;
                                      }
      default :   
        LOG_S(0) << "Not supported type at the port" << "\n";
        break;
    }
  }

  // F is a pointer to a Function instance
  //Iterate over Instructions. Each instruction iterate over 
  for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I)
    llvm::outs() << *I << "\n";
   
}

HdlProperty DataAnalyzer::analyzePointer(llvm::Value* valuePointerTy)
{
  outs() << "Pointer analysis \n";
  bool staticIndex = true;
  int maxIdx = 0;
  const DataLayout & DL = m_irModule->getDataLayout();

  HdlProperty property;
  assert(valuePointerTy->getType()->getPointerElementType()->getTypeID() == llvm::Type::IntegerTyID);
  for(llvm::User * U : valuePointerTy->users())
  {
    outs() << *U << " valueID: " << U->getValueID() << "\n";    
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
                                                      outs() << "Value of operand " << ((llvm::ConstantInt*)val)->getZExtValue() << "\n";
                                                      maxIdx = std::max(maxIdx, (int)((llvm::ConstantInt*)val)->getZExtValue());
                                                    }
                                                          
                                                }
        case llvm::Instruction::Load          : break;
        default : staticIndex = false; outs() << I->getOpcodeName() << " is not supported \n";
      }
    }
  }
  property.vtype = (staticIndex? HdlVectorType::arrayType : HdlVectorType::memoryType);
  if (property.vtype == HdlVectorType::arrayType) {
    property.bitwidth = valuePointerTy->getType()->getPointerElementType()->getIntegerBitWidth();
    property.arraylength = maxIdx + 1;
  } else {
    property.arraylength = DL.getPointerSizeInBits();  
    property.bitwidth    = valuePointerTy->getType()->getPointerElementType()->getIntegerBitWidth();
  }

  outs() << "Indexing is static " << staticIndex << "\n";
  outs() << "--- end analysis \n";
  return property;
} 



