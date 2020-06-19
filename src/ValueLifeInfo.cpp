/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include "ValueLifeInfo.hpp"

using namespace hdbe;


void ValueLifeInfo::setBirthTime(float schedule, float valid) {
  if (schedule > this->schedule.time || valid > this->valid.time) {
    this->schedule.time  = schedule;
    this->valid.time     = valid;
  } else 
    LOG_S(WARN) << *(this->irValue) << ": new birth time is smaller than current value, not updated\n";
};

    
void ValueLifeInfo::addUseTime(float time) { 
  TimePoint tmp = {.time = time};
  lastuse.time = std::max<float>(time, lastuse.time);
  useTimeList.push_back(tmp);
}
    
