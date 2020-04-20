#include "ValueLifeInfo.hpp"

using namespace hdbe;


void ValueLifeInfo::setBirthTime(BasicBlock *bb, float step) {
  birthTime.block = bb; 
  birthTime.step  = step; 
  LOG_S(10) << irValue << " valid on " << bb->getName() << " " << step << "\n";
};
    
void ValueLifeInfo::addUseTime(BasicBlock *bb, float step) { 
  TimePoint tmp = {.block = bb, .step = step};  
  useTimeList.push_back(tmp);
}
    
//Assuming all are in same block
int ValueLifeInfo::getLiveTime() {
  return floor(useTimeList.back().step) - floor(birthTime.step);
}