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
#include "loguru/loguru.hpp"

#include "ControlStep.hpp"
#include "Scheduler.hpp"


using namespace llvm;
using namespace hdlbe;

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
  
  Scheduler& scheduler = *(new AsapScheduler(Mod.get()));
  SchedulingAlgorithm algo;
  scheduler.schedule(algo, "fibo");

  return 0;
}
