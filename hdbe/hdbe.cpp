/* Authors: 
Eli Bendersky (eliben@gmail.com)
Jeff Lieu (lieumychuong@gmail.com)
*/
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
#include "InstructionScheduler.hpp"

using namespace llvm;
using namespace hdbe;

int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " [Options] <IR file> [functionName]\n";
    return 1;
  }
  
  hdbe::g_verbosity = 2;
  
  signal(SIGSEGV, segfault_handler);
  

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
  
  //////////////////////////////////////////////////////
  ///   Control Data Info is the main object that holds 
  /// lots of  information relevant to the HDL generation processes
  ControlDataInfo CDI(Mod.get(), funcName);

  /// Preprocessing the IR data structure
  IRPreprocessor IRPrep(Mod.get(), funcName);
  IRPrep.transformNames();

  /// Perform data analysis
  DataAnalyzer DAnalyzer(&CDI);
  DAnalyzer.analyze();
  
  /// Schedule the instructions
  InstructionScheduler IS(&CDI);
  IS.schedule();

  CDI.dumpStateList();

  /// Then output SystemVerilog entity
  VerilogGenerator VGen(&CDI);
  VGen.write();

  
  
  LOG(INFO, "Program ends .. "); 
  return 0;
}
