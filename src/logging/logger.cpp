
#include "logger.hpp"

int hdbe::g_verbosity = 0;

llvm::raw_ostream& hdbe::os = llvm::outs();

llvm::raw_ostream& hdbe::LOG_S(int level) {
  if (g_verbosity >= level) {
    std::string lvl(level*2, ':');
    return (llvm::outs() << lvl);
  }
  else 
    return llvm::nulls();  
}


