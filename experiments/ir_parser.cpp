//Thanks to Eli Bendersky (eliben@gmail.com) for an initial file that sets up dependency to get me get started quickly
#include <typeinfo> 

#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include "logging/logger.hpp"
#include "ControlDataInfo.hpp"
#include "IRPreprocessor.hpp"
#include "DataAnalyzer.hpp"
#include "CodeGenerator.hpp"

using namespace llvm;
using namespace hdbe;

int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " [Options] <IR file> [functionName]\n";
    return 1;
  }
  
  hdbe::g_verbosity = 6;
  
  LOG(INFO, "Program starts .. ");
 
  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  LLVMContext Context;
  std::unique_ptr<Module> Mod(parseIRFile(argv[1], Err, Context));  
  const char *funcName = argv[2];
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }
  

  ControlDataInfo CDI(Mod.get(), funcName);
  IRPreprocessor IRPrep(Mod.get(), funcName);
  IRPrep.transformNames();
  DataAnalyzer DAnalyzer(&CDI);
  DAnalyzer.analyze();
  VerilogGenerator VGen(&CDI);
  VGen.write();
  
  LOG(INFO, "Program ends .. "); 
  return 0;
}
