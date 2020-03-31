//Thanks to Eli Bendersky for an initial file that sets up dependency to get me get started quickly
//------------------------------------------------------------------------------
// This is currently ad-hoc
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#define println(LN) outs() << LN << '\n'

using namespace llvm;

int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " <IR file>\n";
    return 1;
  }

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  LLVMContext Context;
  std::unique_ptr<Module> Mod(parseIRFile(argv[1], Err, Context));
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  // Go over all named mdnodes in the module
  for (Module::const_iterator I = Mod->begin(),
                                             E = Mod->end();
       I != E; ++I) {
    
    // These dumps only work with LLVM built with a special cmake flag enabling
    // dumps.
    //I->dump();
    outs() << "Found Function: " << I->getName() << '\n';
    println("Arguments :");
    for(Function::const_arg_iterator ai = I->arg_begin(), ae = I->arg_end(); ai != ae; ++ai)
    {
      ai->dump();
    }

    for(Function::const_iterator bbi = I->begin(), bbe = I->end(); bbi != bbe ; ++bbi)
    {
      for(BasicBlock::const_iterator ins_i = bbi->begin(), instruction_end = bbi->end(); ins_i != instruction_end; ++ ins_i)
      {
        ins_i->dump();
      }
    }
  }

  return 0;
}
