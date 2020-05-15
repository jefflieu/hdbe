#include "ValueLifeInfo.hpp"

using namespace hdbe;


void ValueLifeInfo::setBirthTime(HdlState *state, float time) {
  birthTime.state = state; 
  if (time >= birthTime.time)
    birthTime.time  = time;
  else 
    LOG_S(WARN) << *(this->irValue) << ": new birth time is smaller than current value, not updated\n";
};

    
void ValueLifeInfo::addUseTime(HdlState *state, float time) { 
  TimePoint tmp = {.state = state, .time = time};  
  useTimeList.push_back(tmp);
}
    
//Assuming all are in same block
int ValueLifeInfo::getLiveTime() {
  LOG_IF_S(WARN, useTimeList.empty()) << irValue->getName() <<  " variable is not used \n";
  if (useTimeList.empty()) 
    return 0;
  else 
    return useTimeList.back().state->id - floor(birthTime.time);
}