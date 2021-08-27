
#include "logger.hpp"

int hdbe::g_verbosity = 0;
int hdbe::start_of_line = 0;

llvm::raw_ostream& hdbe::_log_stdout = llvm::outs();
llvm::raw_ostream& hdbe::_log_stderr = llvm::errs();

llvm::raw_ostream& hdbe::LOG_S(int level) {
  start_of_line = llvm::outs().tell();
  std::string lvl;
  //llvm::outs().changeColor(Colors::BLACK); 
  llvm::outs().resetColor(); 
                  
  switch(level)
  {
    case FATAL :  lvl = "FATAL :"; 
                  llvm::outs().changeColor(Colors::RED); 
                  break;
    case ERROR :  lvl = "ERROR :"; 
                  llvm::outs().changeColor(Colors::RED); 
                  break;
    case WARN  :  lvl = "WARN  :";
                  llvm::outs().changeColor(Colors::YELLOW); 
                  break;
    case INFO  :  lvl = "INFO  :"; break;
    default    :  lvl = "DEBUG " + std::string(level*2, ':');
  }
  if (g_verbosity >= level) {
    return (llvm::outs() << lvl);
  }
  else 
    return llvm::nulls();  
}

llvm::raw_ostream& hdbe::LOG_PAD(int column) {
  int current_column = llvm::outs().tell() - start_of_line;
  int pad_size = (column - current_column) >=1 ? (column - current_column) : 1;
  std::string pad(pad_size, ' ');
  llvm::outs() << pad;
  return llvm::outs();
}


llvm::raw_ostream& hdbe::ASSERT_S(bool cond) {
  if (! cond) {
    return (llvm::outs() << "ASSERTION FAILED: ");
  }
  else 
    return llvm::nulls();  
}


void hdbe::segfault_handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  _log_stderr << "Error: signal : " << sig << "\n";
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
