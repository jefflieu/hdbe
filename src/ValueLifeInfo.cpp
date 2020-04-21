#include "ValueLifeInfo.hpp"

using namespace hdbe;


void ValueLifeInfo::setBirthTime(HdlState *state, float time) {
  birthTime.state = state; 
  birthTime.time  = time;
};

    
void ValueLifeInfo::addUseTime(HdlState *state, float time) { 
  TimePoint tmp = {.state = state, .time = time};  
  useTimeList.push_back(tmp);
}
    
//Assuming all are in same block
int ValueLifeInfo::getLiveTime() {
  return useTimeList.back().state->id - floor(birthTime.time);
}