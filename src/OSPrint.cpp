
#include "OSPrint.hpp"

using namespace std;
using namespace hdbe;

std::ostream& hdbe::operator<<(std::ostream& os, const ControlStep& cs) {    
  cs.print(os);
  return os;
}; 


