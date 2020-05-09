#pragma once
#include "llvm/Support/raw_ostream.h"
#include <execinfo.h>
#include <csignal>
#include <stdlib.h>
#include <unistd.h>

#define FATAL  -3
#define ERROR  -2 
#define WARN   -1
#define INFO    0 
#define EXTRA   1 
#define DEEP2   2  
#define DEEP3   3 

namespace hdbe {

using Colors = llvm::raw_ostream::Colors;

extern int g_verbosity;
extern int start_of_line;
extern llvm::raw_ostream& _log_stdout;
extern llvm::raw_ostream& _log_stderr;

extern llvm::raw_ostream& LOG_PAD(int column);
extern llvm::raw_ostream& LOG_S(int);
extern llvm::raw_ostream& ASSERT_S(bool);
extern void segfault_handler(int sig);

#define LOG(level, out_msg) do { \
  constexpr int funcLength = sizeof(__FUNCTION__); \
  constexpr int pad = std::max<int>(30-funcLength, 1); std::string space(pad, ' '); \
  LOG_S(level) << __FUNCTION__ << space << " : " << out_msg << "\n";} while(0)

#define ASSERT(cond, message) do {if (!(cond)) {_log_stderr << "ASSERTION FAILED: " << message; assert(cond);}} while(0)

#define LOG_START(level) LOG(level, "Start ...")
#define LOG_DONE(level) LOG(level, "Done!")

#define LOG_ERROR_S (LOG_S(ERROR) << __FUNCTION__ << " , " << __LINE__ << ":")
#define LOG_INFO_S (LOG_S(INFO)  << __FUNCTION__ << " , " << __LINE__ << ":")
#define LOG_WARN_S (LOG_S(WARN)  << __FUNCTION__ << " , " << __LINE__ << ":")

#define LOG_ERROR_IF(cond, message) (LOG_IF_S(ERROR, cond) << __FUNCTION__  << ":" << message << "\n")

#define LOG_IF_S(level, cond) (LOG_S(cond?level:100))
#define LOG_WARN_IF_S(cond) (LOG_IF_S(WARN, cond))  
#define LOG_ERROR_IF_S(cond) (LOG_IF_S(ERROR, cond))  

}