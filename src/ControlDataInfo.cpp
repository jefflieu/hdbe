/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include "ControlDataInfo.hpp"
#include "logging/logger.hpp"

using namespace hdbe;

HdlState& ControlDataInfo::getInstructionState(Instruction* I)
{
  for(HdlState& state : stateList)
  {
    for(Instruction* inst : state.instructionList)
    {
      if (I == inst)
      {
        return state;
      }
    }
  }
  LOG_S(FATAL) << "Instruction has not been scheduled\n";
  assert(0);
}

//Simple loop to search 
HdlCFGEdge& ControlDataInfo::findCFGEdge(BasicBlock* src, BasicBlock* dst)
{
  for(HdlCFGEdge &E : transitionList)
  {
    if (E.getSrcBB() == src && E.getDestBB() == dst) return E;
  }
  LOG_S(FATAL) << "The edge :" << src->getName().str() << "->" << dst->getName().str() << " can't be found \n";
  assert(0);  
}


//Simple loop to search 
CFGEdgeVector ControlDataInfo::findAllCFGEdges(BasicBlock* src, BasicBlock* dst)
{
  CFGEdgeVector edgeVector;
  for(auto E : transitionList)
  {
    if (E.getSrcBB() == src && E.getDestBB() == dst) edgeVector.push_back(&E);
  }
  
  LOG_IF_S(FATAL, edgeVector.empty()) << "The edge :" << src->getName().str() << "->" << dst->getName().str() << " can't be found \n";
  assert(!edgeVector.empty());
  return edgeVector;
}

    

void ControlDataInfo::addCFGEdge(HdlCFGEdge& edge)
{
  //NONE
}


void ControlDataInfo::dumpStateList()
{
  LOG_S(0) << "---------------------------------" << "\n";
  LOG_S(0) << "Schedule" << "\n";

  for(HdlState& state : this->stateList)
  {
      state.dump();
      if (state.isLast())
        LOG_S(0) << "Latency :" << state.id << "\n";
  }
  LOG_S(0) <<"---------------------------------" << "\n";
}


void ControlDataInfo::dumpValueInfoMap()
{
  LOG_S(0) <<"---------------------------------" << "\n";
  LOG_S(0) <<"ValueInfoMap" << "\n";
  for(auto item : this->valueInfoMap)
  {
    LOG_S(0) << item.second.repr();
  }
  LOG_S(0) <<"---------------------------------" << "\n";
}