#pragma once 
#include <string> 
#include <list> 
#include <stdint.h> 
#include <iostream>

#include "BaseClass.hpp"
#include "types.hpp"

namespace hdlbe {

class SchedulingAlgorithm;

class Scheduler : public BaseClass {
  friend SchedulingAlgorithm;
  protected:     
    Module_h m_irModule;
    std::list<ControlStep_h> m_ctrlSteps;
    
  public: 
    Scheduler (Module_h _irModule) : m_irModule(_irModule) {};
    ~Scheduler() {};

    virtual uint32_t schedule(SchedulingAlgorithm& algo, std::string funcName){};
  
};

class AsapScheduler : public Scheduler {
    
  public:     
    AsapScheduler (Module_h _irModule) : Scheduler(_irModule) {};
    ~AsapScheduler() {};

    uint32_t schedule(SchedulingAlgorithm& algo, std::string funcName) override;
  
};


class SchedulingAlgorithm {
  
  public:
    SchedulingAlgorithm () {};
    ~SchedulingAlgorithm () {};
    
    uint32_t visit(AsapScheduler* scheduler, std::string funcName);
};

}


