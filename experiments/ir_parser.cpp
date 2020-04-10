//Thanks to Eli Bendersky for an initial file that sets up dependency to get me get started quickly
//------------------------------------------------------------------------------
// This is currently ad-hoc
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <typeinfo> 

#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CallGraph.h"
#include "loguru/loguru.hpp"
#include "ControlStep.hpp"
#include "Scheduler.hpp"
#include "CodeGenerator.hpp"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DDG.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;
using namespace hdbe;

int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " [Options] <IR file> [functionName]\n";
    return 1;
  }
  
  /*Setup log from Loguru */
  // Only show most relevant things on stderr:
  loguru::g_internal_verbosity = loguru::Verbosity_MAX;
  loguru::g_stderr_verbosity = 1;
  loguru::g_preamble_date    = 0;
  loguru::g_preamble_time    = 1;
  loguru::g_preamble_uptime  = 0;
  loguru::g_preamble_thread  = 0;
  loguru::g_preamble_file    = 1;

  // Put every log message in "everything.log":
  loguru::init(argc, argv);
  loguru::add_file("everything.log", loguru::Append, loguru::Verbosity_MAX);
  // Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
  loguru::add_file("latest_readable.log", loguru::Truncate, loguru::Verbosity_INFO);



  //LOG_SCOPE_F(INFO, "Will indent all log messages within this scope.");
  //LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
  //LOG_F(2, "Will only show if verbosity is 2 or higher");
  //VLOG_F(1, "Use vlog for dynamic log level (integer in the range 0-9, inclusive)");
  //LOG_IF_F(ERROR, 1, "Will only show if badness happens");

  // Throw exceptions instead of aborting on CHECK fails:
  loguru::set_fatal_handler([](const loguru::Message& message){
	throw std::runtime_error(std::string(message.prefix) + message.message);
  });

  LOG_F(INFO, "Program starts .. "); 
  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  LLVMContext Context;
  std::unique_ptr<Module> Mod(parseIRFile(argv[1], Err, Context));  
  std::string  funcName = argv[2];
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }
  
  Module_h mod_h = Mod.get();
  for(auto global_var = mod_h->global_begin(), last = mod_h->global_end(); global_var != last; ++global_var)
  {
    LOG_S(1) << g_getStdStringName(*global_var);
    global_var -> dump();
  }
  
  
  //CallGraph callGraph(*Mod);
  //Function* func = Mod->getFunction(funcName);  
  
  
  SimpleScheduler scheduler(Mod.get());
  SchedulingAlgorithm algo;
  scheduler.schedule(algo, funcName);
  VhdlGenerator vhdlGen(&scheduler);
  vhdlGen.write();
  
  
  LOG_F(INFO, "Program ends .. "); 
  return 0;
}
