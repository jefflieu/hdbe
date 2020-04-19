
#include "OSPrint.hpp"
#include "llvm/ADT/StringRef.h"

using namespace std;
using namespace hdbe;

std::ostream& hdbe::operator<<(std::ostream& os, const ControlStep& cs) {    
  cs.print(os);
  return os;
}; 

std::string hdbe::to_hexstring(uintptr_t ptr, char h) {
  std::string s;  
  const char upper[]= {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  const char lower[]= {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'F'};
  for(uint32_t i = sizeof(ptr)*2; i > 0; i--)
  {
    s.insert(0,1, (h=='H'?upper[ptr & 0xf]:lower[ptr & 0xf]));
    ptr = ptr >> 4;
  }
  return s;
}

