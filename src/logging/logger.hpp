#pragma once
#include "llvm/Support/raw_ostream.h"

#define FATAL  -3
#define ERROR  -2 
#define WARN   -1
#define INFO    0 
#define EXTRA   1 
#define DEEP2   2 
#define DEEP3   3 

namespace hdbe {
extern int g_verbosity;
extern llvm::raw_ostream& _log_stdout;
extern llvm::raw_ostream& LOG_S(int);


#define LOG(level, out_msg) do {std::string space(20-sizeof(__FUNCTION__), ' '); \
  if (g_verbosity >= level) _log_stdout << __FUNCTION__ << space << " : " << out_msg << "\n"; } while(0)


}