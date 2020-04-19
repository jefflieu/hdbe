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
using string = std::string;

void IRPreprocessor::transformNames()
{
  for (llvm::inst_iterator I = inst_begin(irFunction), E = inst_end(irFunction); I != E; ++I)
  {
    string newName = makeHdlName(I->getName().str());
    I->setName(newName);
    LOG_S(IR_PP_DBG) << I->getName() << "\n";
  }
}