#include <list>
#include <string>
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"


#include "IRPreprocessor.hpp"
#include "HDLUtil.hpp"

#define IR_PP_DBG 10

using namespace hdbe;

using Function = llvm::Function;
using Instruction = llvm::Instruction;
using llvm::outs;
using StringRef = llvm::StringRef;
using String = std::string;

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
    for (llvm::inst_iterator I = inst_begin(&*F), E = inst_end(&*F); I != E; ++I)
    {
      if (I->getType()->isVoidTy()) continue;
      String newName = makeHdlName(I->getName().str());
      I->setName(newName);
      LOG_S(IR_PP_DBG) << I->getName() << "\n";
    }
  }
}