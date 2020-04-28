
#include "logger.hpp"

int hdbe::g_verbosity = 0;

llvm::raw_ostream& hdbe::_log_stdout = llvm::outs();
llvm::raw_ostream& hdbe::_log_stderr = llvm::errs();

llvm::raw_ostream& hdbe::LOG_S(int level) {
  if (g_verbosity >= level) {
    if (level >= 0) {
      std::string lvl(level*2, ':');
      return (llvm::outs() << lvl);
    } else {
      return (llvm::outs() << "***");
    }
  }
  else 
    return llvm::nulls();  
}


llvm::raw_ostream& hdbe::ASSERT_S(bool cond) {
  if (! cond) {
    return (llvm::outs() << "ASSERTION FAILED: ");
  }
  else 
    return llvm::nulls();  
}