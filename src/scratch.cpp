uint32_t SchedulingAlgorithm::visit(SimpleScheduler* scheduler, std::string funcName) 
{
  LOG_F(INFO, "SimpleScheduler Visit");  
  
  //Walk the dependence graph and map instruction to "time"      
  std::map<llvm::DDGNode*, int> nodeWeightMap;
  std::map<llvm::Instruction*, int> instructionSchedule;
  llvm::DDGNode &root = scheduler->DDG->getRoot();
  nodeWeightMap[&root] = 0;
  for(auto I = bf_begin(scheduler->DDG); I != bf_end(scheduler->DDG); I++)
  {
      int32_t nodeLatency = nodeWeightMap[*I];      
      if (I->getKind() == llvm::DDGNode::NodeKind::SingleInstruction || I->getKind() == llvm::DDGNode::NodeKind::MultiInstruction)
      {
        auto& InstList = (static_cast<llvm::SimpleDDGNode*>(*I))->getInstructions();
        //Iterate around InstList 
        for(auto inst = InstList.begin(), last = InstList.end(); inst != last; ++inst)
          {
            instructionSchedule[*inst] = nodeLatency;
            nodeLatency += 1;
          }
        LOG_S(1) << "Node " << *I << " has " << InstList.size() << " instructions " ;  
      }
      else {
        LOG_S(1) << "Node " << *I << " has no instructions";        
      }
            
      //Iterate TargetNode 
      for(auto edge_i = I->begin(); edge_i != I->end(); edge_i ++ )
      {
        llvm::DDGNode &N = (*edge_i)->getTargetNode();
        auto newWeight = nodeWeightMap[*I] + nodeLatency;
        nodeWeightMap[&N] = newWeight > nodeWeightMap[&N]? newWeight: nodeWeightMap[&N];
      }
      
  }
  
  for(auto I = nodeWeightMap.begin(); I != nodeWeightMap.end(); I++)
  {
    LOG_S(1) <<"Node " << I->first << " has weight " << I->second;
  }

  for(auto I = instructionSchedule.begin(); I != instructionSchedule.end(); I++)
  {
    LOG_S(1) <<"Instruction " << I->first->getOpcodeName() << " is scheduled as " << I->second;
  }
    
  try {       
    
    Function_h F = scheduler->m_module->getFunction(funcName);
    LOG_IF_S(FATAL, F == NULL) << "Function not found";
    LOG_S(INFO) << "Found Function: " << GetName(F);    
    LOG_S(INFO) << "Arguments :";      
    
    for(auto bbi = F->begin(), bbe = F->end(); bbi != bbe ; ++bbi)
    {
      LOG_S(6) << " Basic block: " << GetName(*bbi);
      std::list<Const_Instruction_h> instructions;                
      
      //Collecting all instructions in the basic block 
      for(auto ins_i = bbi->begin(), instruction_end = bbi->end(); ins_i != instruction_end; ++ ins_i)
      {
        auto ins_h = D_GET_PTR(ins_i);
        instructions.push_back(ins_h);
        LOG_S(6) << "Push to instruction list " << GetName(ins_h) <<" ID: " << ins_h;        
        for(auto op_i = ins_h->op_begin(), last = ins_h->op_end(); op_i != last; op_i++)
        {
          llvm::outs()<<(*op_i)->getName()<< " ID: " << (*op_i) <<'\n';
        }
        //At the same time work out who depens on this 
        //for(auto ins_j = bbi->begin(), last = bbi->end(); ins_j != last; ++ ins_j)
        //{
        //  
        //}
      }

      //Scheduling 
      uint32_t id = 0;
      while( ! instructions.empty()) {
        const std::string step_name = GetName(*bbi) + "." + std::to_string(id);
        ControlStep *cs  = new ControlStep(step_name);
        LOG_S(6) << step_name;
        Const_Instruction_h I =  instructions.front();
        ///I->dump();
        instructions.pop_front();
        cs->addInstruction(I);
        scheduler->addControlStep(cs);
        id ++ ;
        //I->dump();
        ////For each instruction already in the ControlStep check         
        ////for(auto list_iter = instructions.begin(); list_iter != instructions.end(); list_iter++)
        ////  {
        ////    
        ////  }
      }
    }
  }
  catch (...) {    
  }
}

