
/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

#include "CodeGenerator.hpp"
#include "VerilogTemplate.hpp"
#include "logging/logger.hpp"
#include "IRUtil.hpp"

#ifndef VG_DBG 
#define VG_DBG 9
#endif 

using namespace hdbe;

using String      = std::string;
using Value       = llvm::Value;
using User        = llvm::User;
using Instruction = llvm::Instruction;
using Twine       = llvm::Twine;

void VerilogGenerator::write() 
{
  auto F         = CDI_h->irFunction;
  String filename = F->getName().str() + ".sv";
  std::ofstream os(filename, std::ofstream::out);  

  LOG_START(VG_DBG);

  os << VERILOG_DECL_MODULE(F->getName().str());

  writePorts(os);
  
  writeSignalDeclaration(os);
  
  writeInputAssignment(os);
  
  writeCtrlFlow(os);
  
  writeInstructions(os);
  
  writeRegisterStages(os);
  
  writeReturnStatement(os);
  
  writeArrayObject(os);
  
  writeVCDLogging(os);

  os << VERILOG_DECL_MODULE_END(F->getName().str());    
  os.close();  
};


/// Concept of tagging of a variable: 
/// Each HDL variable corresponds to one or many value in IR
/// In order to pipeline the dataflow, we introduce "tag"s to variable 
/// A variable has _n tag which indicates the clock cycle in which it's valid 
/// For example, an instruction is executed at clock cycle 3: a <= b + c 
/// The instruction is described as following in HDL: 
///   a_3 <= b_3 + c_3;
/// This tagging mechanism helps us to pipeline the dataflow and invoke the function repeatedly every clock cycle

std::ostream& VerilogGenerator::writeSignalDeclaration(std::ostream& os)
{  
  auto &variableList    = CDI_h->variableList;
  auto &VIM             = CDI_h->valueInfoMap;
  auto &memObjList      = CDI_h->memObjList;
  auto &transitionList  = CDI_h->transitionList;
  LOG_START(VG_DBG);

  HDLConcurrentBlock declBlock("Signal Declaration");
  //Starting from variableList
  for(auto I = variableList.begin(), E = variableList.end(); I!=E; ++I)
  {
    HdlVariable &var = *I;
    //for each signal, get live time 
    LOG_S(VG_DBG) << *(var.getIrValue()) << "\n";
    int valid_time  = floor(VIM[var.getIrValue()].valid.time);
    int use_time    = std::max<int>(floor(VIM[var.getIrValue()].getLatestUseTime()), valid_time) ;

    LOG_S(VG_DBG + 1) << " Timing info " << valid_time << " " << use_time << "\n";
    
    for(uint32_t i = valid_time; i <= use_time; i++)
    {
      String tag = CYCLE_TAG(i);
      declBlock.addStatement(new HDLExpression(writeHdlObjDeclaration(var, tag)));
    }
    
    if (var.property.isBackValue)
    {
      declBlock.addStatement(new HDLExpression(writeHdlObjDeclaration(var, LOOP_TAG)));
    }

  }

  //Memory objects list including array type 
  for(auto I = memObjList.begin(), E = memObjList.end(); I!=E; ++I)
  {
    HdlMemory &var = *I;
    if (var.property.stype == HdlSignalType::regType)
      declBlock.addStatement(new HDLExpression(writeHdlObjDeclaration(var, "")));      
  }

  //State transition list 
  declBlock.addStatement(new HDLCommentLine("State transition"));

  for(auto E = transitionList.begin(); E != transitionList.end(); ++E)
  {
    HdlCFGEdge &var = *E;
    //for each signal, get live time 
    LOG_S(VG_DBG) << *(var.getIrValue()) << "\n";
    int valid_time = floor(VIM[var.getIrValue()].valid.time);
    int use_time   = std::max<int>(floor(VIM[var.getDestBB()].getLatestUseTime()), valid_time) ;
    LOG_S(VG_DBG + 1) << " Timing info " << valid_time << " " << use_time << "\n";
    
    for(uint32_t i = valid_time; i <= use_time; i++)
    {
      String tag = CYCLE_TAG(i);
      declBlock.addStatement(new HDLExpression(writeHdlObjDeclaration(var, tag)));
    }
    if (var.isBackEdge())
    {
      declBlock.addStatement(new HDLExpression(writeHdlObjDeclaration(var, LOOP_TAG)));
    } 
  }
  os << declBlock.generateCode(this);
}


std::ostream& VerilogGenerator::writePorts(std::ostream& os){
  auto &portList = CDI_h->portList;
  auto F         = CDI_h->irFunction;
  HDLConcurrentBlock portDecl("Port declarations");

  LOG_START(VG_DBG);
  
  for(auto I = portList.begin(), E = portList.end(); I!=E; ++I)
  {
    HdlPort& port = *I;
    portDecl.addStatement(new HDLExpression(writeHdlObjDeclaration(port)));
  }
  portDecl.addStatement(new HDLExpression(VERILOG_HDBE_PORTS));

  os << portDecl.generateCode(this);
  return os;
};


///This is the main process that spawns out various processes which are specialized for each type of instructions 
std::ostream& VerilogGenerator::writeInstructions(std::ostream& os){
  
  std::list<HdlVariable> &variableList = CDI_h->variableList;
  
  os << VERILOG_CODE_SECTION("Instructions");

  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    //
    if (llvm::Instruction::classof(var_i->getIrValue())) {
      
      auto instr = static_cast<Instruction*>(var_i->getIrValue());
      if (isMemoryInstruction(instr)) {
        //Do nothing yet
      } else if (isPHIInstruction(instr)) {

        os << writePHIInstruction(instr);

      } else {

        os << writeSimpleInstruction(instr);      

      }
    }
  }  
  
  return os;
};

std::ostream& VerilogGenerator::writeCtrlFlow(std::ostream& os )
{
  auto F  = CDI_h->irFunction;
  auto TL = CDI_h->transitionList;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  String statement;
  LOG_START(VG_DBG);
  HDLConcurrentBlock ctrl_flow_code("Control Flow");
  HDLAssign * fc_statement;
  
  for(auto &E : TL)
  {
    //For each transition edge we write the valid condition of it 
    int valid_time = floor(VIM[static_cast<Value*>(E.getIrValue())].valid.time);
    HDLIdentifier dst = E.getName().str() + CYCLE_TAG(valid_time);
    HDLIdentifier src = writeControlActiveCondition(static_cast<Instruction*>(E.getIrValue()), E.getDestBB(), valid_time);
    
    //Make a new assignment and add to the code block
    fc_statement = new HDLConcurrentAssign(dst, src);
    ctrl_flow_code.addStatement(fc_statement);
  }


  for(auto bb_iter = F->begin(), bb_end = F->end(); bb_iter != bb_end; bb_iter ++ )
  {
    BasicBlock& bb = *bb_iter;
    int schedule_time = floor(VIM[static_cast<Value*>(&bb)].getScheduledTime());
    String activ_tag  = CYCLE_TAG(schedule_time);
    HDLIdentifier basicblock_net = bb.getName().str() + activ_tag;
    
    //Work out how each valid block is valid and add the assignment the code block
    fc_statement = new HDLConcurrentAssign(basicblock_net, '|');
    if (bb.hasNPredecessors(0))
    {
      fc_statement->setRHS(FUNC_START_NET);
    } else {
      int cnt = 0;
      for(auto E : TL)
      {
        if (E.getDestBB() == &bb)
        {
           String backedgeTag = LOOP_TAG;
           HDLIdentifier src = E.getName().str() + (E.isBackEdge()?backedgeTag:activ_tag);
           fc_statement->setRHS(src);
        }
      }
    }
    ctrl_flow_code.addStatement(fc_statement);
  }
  os << ctrl_flow_code.generateCode(this);
  LOG_DONE(VG_DBG);
  return os;
}

String VerilogGenerator::writeControlActiveCondition(llvm::Instruction* I, llvm::BasicBlock *bb, int id)
{
  String condition;
  char buf[256];
  unsigned size = 0;
  String tag = CYCLE_TAG(id);
  String defltCondition("!(");
  if (I->getOpcode() == llvm::Instruction::Switch) {    
    //Search for the basic block
    for(int i = 2; i < I->getNumOperands(); i+=2) {
      String cond = "(" + getValueHdlName(I->getOperand(0)) + tag + VERILOG_COMPARE_EQUAL + getValueHdlName(I->getOperand(i)) + tag 
            + VERILOG_LOGICAL_AND + getValueHdlName(I->getParent()) + tag + ")";
      
      if(bb == I->getOperand(i+1))
        condition += cond; 
      defltCondition += cond + ((i+2<I->getNumOperands() )?String("|"):String(")"));    
    }
    defltCondition += VERILOG_LOGICAL_AND + getValueHdlName(I->getParent()) + tag ;
  } else if (I->getOpcode() == llvm::Instruction::Br) {
    if (static_cast<llvm::BranchInst*>(I)->isConditional())
    {
      if (bb == I->getOperand(1))
        condition += "(!" + I->getOperand(0)->getName().str() + tag + VERILOG_LOGICAL_AND + getValueHdlName(I->getParent()) + tag + ")";
      else 
        condition += "(" + I->getOperand(0)->getName().str() + tag + VERILOG_LOGICAL_AND + getValueHdlName(I->getParent()) + tag + ")";
    } else {
      condition += "(" + getValueHdlName(I->getParent()) + tag + ")";
    }
  }

  if (condition.size()) return condition;
  else return defltCondition;
}

String VerilogGenerator::writePHIInstruction(llvm::Instruction* I)
{
  String space(50, ' ');
  String instantiate;
  String loop_assign;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  int schedule_time = floor(VIM[static_cast<Value*>(I)].getScheduledTime());
  int valid_time    = floor(VIM[static_cast<Value*>(I)].getValidTime());
  size = sprintf(buf, "\"%8s\"", I->getOpcodeName());
  String opcodeString = String(buf, size);

  LOG_IF_S(ERROR, ! llvm::PHINode::classof(I)) << "Error, instruction is not PhiNode\n";
  LOG_IF_S(ERROR, ! I->getType()->isIntegerTy()) << "Error, PHINode is not integer type\n";
  assert (llvm::PHINode::classof(I));    
  assert (I->getType()->isIntegerTy());

  auto phi = static_cast<llvm::PHINode*>(I);
  auto N = phi->getNumIncomingValues();
  HDLInstance phiInstance("PHINodeOp", reinterpret_cast<uintptr_t>(I));
  phiInstance.setParam(static_cast<llvm::PHINode*>(I)->getNumIncomingValues());
  phiInstance.setParam(phi->getType()->getIntegerBitWidth());
  phiInstance.setPort(FUNC_CLK_NET);
  String activeCycle = I->getParent()->getName().str() + CYCLE_TAG(schedule_time);
  phiInstance.setPort(activeCycle);

  String output = getValueHdlName(I) + CYCLE_TAG(valid_time);
  phiInstance.setPort(output);
  for(int i = 0; i < N; i++)
  {
    llvm::BasicBlock* blk = phi->getIncomingBlock(i);
    llvm::Value* val = phi->getIncomingValue(i);
    HdlCFGEdge& edge = CDI_h->findCFGEdge(blk, phi->getParent());
    String edgeName = edge.getName().str();
    String valName = getValueHdlName(val);
    if (!edge.isBackEdge()) {
      edgeName += CYCLE_TAG(schedule_time);
    } else {
      edgeName += LOOP_TAG;
    }
    if (!edge.isBackEdge() || !llvm::Instruction::classof(val)) {
      valName += CYCLE_TAG(schedule_time);
    } else {
      
      int edgeValidTime = floor(VIM[edge.getIrValue()].getValidTime());
      valName += LOOP_TAG;
      HDLConcurrentAssign loop_variable_assign(valName);
      loop_variable_assign.setRHS(getValueHdlName(val) + CYCLE_TAG(edgeValidTime + 1));
      loop_assign += loop_variable_assign.generateCode(this);
    
    }
    String port = "{" + edgeName + "," + valName + "}";
    phiInstance.setPort(port);
  }
  phiInstance.setPortOthers(0, 16 - N);
 
  instantiate  = phiInstance.generateCode(this);
  instantiate += loop_assign; 
  return instantiate;  
} 

String VerilogGenerator::writeSimpleInstruction(llvm::Instruction* I)
{
  std::unique_ptr<HDLInstance> instance;
  String instantiate;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  int schedule_time = floor(VIM[static_cast<Value*>(I)].getScheduledTime());
  float valid_time = VIM[static_cast<Value*>(I)].valid.time;
  size = sprintf(buf, "\"%8s\"", I->getOpcodeName());
  String opcodeString = String(buf, size);
  
  /** 
    The common structure of those components are: 
      Parameter: Opcode
      Parameter: BithWidth 
      Ports : Clk, Enable, Input0 ... Inputn, Output
  */
  if (llvm::BinaryOperator::classof(I)) {    
    instance = std::make_unique<HDLInstance>("BinaryOp", reinterpret_cast<uintptr_t>(I));
    instance->setParam(opcodeString);
    instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());

  } else if (llvm::CmpInst::classof(I)) {
    instance = std::make_unique<HDLInstance>( ( String(I->getOpcodeName()) + "Op"), reinterpret_cast<uintptr_t>(I));
    Twine predNameString = "\"" + llvm::CmpInst::getPredicateName( (static_cast<llvm::CmpInst*>(I))->getPredicate()) + "\"";
    instance->setParam(predNameString.str());
    instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());

  } else if (llvm::CastInst::classof(I)) {
    instance = std::make_unique<HDLInstance>( ("CastOp"), reinterpret_cast<uintptr_t>(I));
    instance->setParam(opcodeString);
    instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());
    instance->setParam(I->getType()->getIntegerBitWidth());
  
  } else if (llvm::PHINode::classof(I) || llvm::SwitchInst::classof(I) ||llvm::BranchInst::classof(I)) {
     return String("/* empty */");
  
  } else {  
    instance = std::make_unique<HDLInstance>( (String(I->getOpcodeName()) + "Op"), reinterpret_cast<uintptr_t>(I));
    instance->setParam(opcodeString);
    //instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());
    instance->setParam(I->getType()->getIntegerBitWidth()); 
  }  
  
  instance->setPort(FUNC_CLK_NET);
  
  String activeCycle = I->getParent()->getName().str() + CYCLE_TAG(schedule_time);
  instance->setPort(activeCycle);
  
  for(const llvm::Use &use : I->operands())
    { 
      String input_net = getValueHdlName(use.get()) + CYCLE_TAG(schedule_time);
      instance->setPort(input_net);
    }

  String output_net = getValueHdlName(I) + CYCLE_TAG(static_cast<int>(valid_time));
  instance->setPort(output_net);

  return instance->generateCode(this);  
};

Ostream& VerilogGenerator::writeRegisterStages(Ostream& os)
{
  String code_txt;
  String loop_variable_assign;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  std::list<HdlVariable>& variableList       = CDI_h->variableList;
  std::list<HdlCFGEdge >& transitionList     = CDI_h->transitionList;

  HDLSequentialBlock registerBlock("register_stage", FUNC_CLK_NET); 

  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.isConstant) continue;
    int valid_time = floor(VIM[var_i->getIrValue()].getValidTime());
    int use_time   = floor(VIM[var_i->getIrValue()].getLatestUseTime());
    for(int i = valid_time + 1; i <= use_time; ++i)
    {
      //assign += var_i->name + CYCLE_TAG(i) + VERILOG_ASSIGN + var_i->name + CYCLE_TAG(i-1) + VERILOG_ENDL;
      HDLIdentifier dst = var_i->name + CYCLE_TAG(i);
      HDLIdentifier src = var_i->name + CYCLE_TAG(i-1);
      HDLNonblockingAssign* assignment = new HDLNonblockingAssign(dst, src);
      registerBlock.addStatement(assignment);
    }

  }

  HDLSequentialBlock transitionBlock("transition_pipeline", FUNC_CLK_NET); 
  for(auto var_i = transitionList.begin(), var_last = transitionList.end(); var_i != var_last; ++var_i)
  {
    //for each signal, get live time     
    int valid_time  = floor(VIM[var_i->getIrValue()].getValidTime());
    int use_time    = floor(VIM[var_i->getDestBB()].getLatestUseTime());    
    for(uint32_t i = valid_time + 1; i <= use_time; i++)
      {
        String tag1 = CYCLE_TAG(i);
        String tag0 = CYCLE_TAG(i-1);
        HDLIdentifier dst = var_i->name + tag1;
        HDLIdentifier src = var_i->name + tag0;
        HDLNonblockingAssign* transition_assignment = new HDLNonblockingAssign(dst, src);
        transitionBlock.addStatement(transition_assignment);
      }
    if (var_i->isBackEdge())
    {
      HDLIdentifier dst = var_i->name + LOOP_TAG;
      HDLIdentifier src = var_i->name + CYCLE_TAG(valid_time);
      HDLNonblockingAssign *assign_loop_identifier = new HDLNonblockingAssign(dst, src);
      transitionBlock.addStatement(assign_loop_identifier);
    }
  }

  code_txt += registerBlock.generateCode(this);
  code_txt += transitionBlock.generateCode(this);
  os << code_txt;
  return os;
}

Ostream& VerilogGenerator::writeInputAssignment(Ostream& os)
{
  std::list<HdlPort>& portList       = CDI_h->portList;
  
  HDLConcurrentBlock inputAssignment("Copying inputs");
  for(auto var_i = portList.begin(), var_last = portList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.stype != HdlSignalType::inputType) continue;
    if (var_i->property.vtype == HdlVectorType::memoryType) continue;
    if (var_i->property.isUnused) continue;
    String tag0 = "_0";
    HDLIdentifier dst = var_i->name + tag0;
    HDLIdentifier src = var_i->name;
    inputAssignment.addStatement( new HDLConcurrentAssign(dst, src));
  } 
  os << inputAssignment.generateCode(this);
  return os;
}

Ostream& VerilogGenerator::writeReturnStatement(Ostream& os)
{
  auto &stateList = CDI_h->stateList;
  HDLConcurrentBlock ret_handling("return handling");
  HDLStatement* comb_return_statement;
  bool ret_void = false;
  for(auto& state : stateList)
  {    
    if (state.isBranch()) {
      if (state.isReturn()) {
        String tag = CYCLE_TAG(state.id);
        if (state.termInstruction != nullptr) {
          HDLIdentifier bbName  = state.termInstruction->getParent()->getName().str() + tag;
          comb_return_statement = new HDLConcurrentAssign(FUNC_DONE_NET, bbName);
        } else {
          HDLIdentifier stateName = state.getName().str();
          comb_return_statement = new HDLConcurrentAssign(FUNC_DONE_NET, stateName);
        }
        ret_handling.addStatement(comb_return_statement);

        //If there is a value to be returned
        if (state.termInstruction != nullptr && state.termInstruction->getNumOperands() > 0) {
          HDLIdentifier ret_value = state.termInstruction->getOperand(0)->getName().str() + tag;
          comb_return_statement = new HDLConcurrentAssign(FUNC_RET_NET, ret_value);
          ret_handling.addStatement(comb_return_statement);
        } else {
          ret_void = true;
        }
      }
    }
  }

  if (false  && (!ret_void)) {
    //Register stage at the return function
  } else {
    //No register at the return
    os << ret_handling.generateCode(this);
  }
  return os;
}





Ostream& VerilogGenerator::writeVCDLogging(Ostream& os)
{
// Print some stuff as an example
char buf[512];
auto F = CDI_h->irFunction;
auto fileName = F->getName().str().data();
int size;

size = sprintf(buf,"\n\n\
initial begin \n\
  if ($test$plusargs(\"trace\") != 0) begin \n\
    $display(\"[%%0t] Tracing to logs/%s.vcd...\", $time); \n\
    $dumpfile(\"logs/%s.vcd\"); \n\
    $dumpvars(); \n\
  end \n\
  $display(\"[%%0t] Model running...\", $time);\n\
end\n\n", 
  fileName, fileName
  );
  String str = String(buf, size);  
  return os  << str;
}

Ostream& VerilogGenerator::writeArrayObject(Ostream &os){
  
  auto &memObjList   = CDI_h->memObjList;
 
  os << VERILOG_CODE_SECTION("Array and memory load/store");
  for(auto I = memObjList.begin(), E = memObjList.end(); I!=E; ++I)
  {
    HdlMemory &memObj = *I;
    if (memObj.property.vtype == HdlVectorType::arrayType)
      os << writeArrayObject(memObj);
    else
      os << writeMemoryObject(memObj);     
  }

  return os;
}

String VerilogGenerator::writeArrayObject(HdlMemory &memObj)
{
  String code_txt;
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  
  HDLConcurrentBlock array_handling("Array handling");
  auto seqBlk = array_handling.addStatement(new HDLSequentialBlock("Store handling", FUNC_CLK_NET));
  HDLSequentialBlock& store_handling = *(static_cast<HDLSequentialBlock*>(seqBlk));


  for(auto instr_i : memObj.memInstrList)
  {
    int activ_time = floor(VIM[static_cast<Value*>(instr_i)].getScheduledTime());
    int valid_time = floor(VIM[static_cast<Value*>(instr_i)].getValidTime());
    String activ_tag = CYCLE_TAG(activ_time);
    String valid_tag = CYCLE_TAG(valid_time);
    int idx = computeIndex(instr_i, memObj.getIrValue());

    if (instr_i->getOpcode() == llvm::Instruction::Load){
      
      HDLIdentifier output = getValueHdlName(instr_i) + valid_tag;
      auto *load_assign_statement = new HDLConcurrentAssign(output);

      if (memObj.property.stype == HdlSignalType::inputType)
        {
          HDLIdentifier input = VERILOG_ARRAY_INDEX(memObj.name + activ_tag, idx);
          load_assign_statement->setRHS(input); 
        }
      else 
        {
          HDLIdentifier input = VERILOG_ARRAY_INDEX(memObj.name, idx);
          load_assign_statement->setRHS(input); 
        }
        array_handling.addStatement(load_assign_statement);
    }
  
    if (instr_i->getOpcode() == llvm::Instruction::Store)
    {
      
      HDLIdentifier activeCycle = instr_i->getParent()->getName().str() + activ_tag;
      HDLIdentifier dst = VERILOG_ARRAY_INDEX(memObj.name, idx);
      HDLIdentifier src = getValueHdlName(instr_i->getOperand(0)) + activ_tag;
      auto * array_assign = new HDLNonblockingAssign(dst, src);
      auto * ifthen       = new HDLIfThenElse(activeCycle);
      ifthen->addThenStatement(array_assign);

      HDLIdentifier valid_flag = VERILOG_ARRAY_INDEX(memObj.name + String(VALID_TAG), idx);
      auto * valid_assign = new HDLNonblockingAssign(valid_flag, activeCycle);


      store_handling.addStatement(ifthen);  
      if (memObj.property.stype == HdlSignalType::outputType)
        store_handling.addStatement(valid_assign);  

    }
  }

  code_txt += array_handling.generateCode(this);
  //code_txt += store_handling.generateCode(this);
  return code_txt;
}

String VerilogGenerator::writeMemoryObject(HdlMemory &memObj)
{
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  unsigned ptrSize   = CDI_h->getPointerSizeInBits();
  unsigned ld_cnt = 0;
  unsigned st_cnt = 0;
  char buf[256];
  unsigned size = 0;

  HDLConcurrentBlock memoryBlock(memObj.name);
  /* 
  The following are involved for 1 memory object: 
  - Read address muxing block
  - Write address muxing block 
  - Write data muxing block 
  - Read enable flag calculation
  - Write enable flag calculation 
  - Read/Write pointers calculation (GEP instruction)
  */

  HDLInstance *load_raddr_instance = new HDLInstance( String("DataMux"), String("RAddr_of_" + memObj.name)); 
  load_raddr_instance->setPort(String(FUNC_CLK_NET));
  load_raddr_instance->setPort(String(MEMOBJ_RADDR(memObj)));
  
  HDLInstance *store_waddr_instance = new HDLInstance( String("DataMux"), String("WAddr_of_" + memObj.name)); 
  store_waddr_instance->setPort(String(FUNC_CLK_NET));
  store_waddr_instance->setPort(String(MEMOBJ_WADDR(memObj)));

  HDLInstance *store_wdata_instance = new HDLInstance( String("DataMux"), String("WData_of_" + memObj.name)); 
  store_wdata_instance->setPort(String(FUNC_CLK_NET));
  store_wdata_instance->setPort(String(MEMOBJ_WDATA(memObj)));

  HDLConcurrentAssign *rden_assignment = new HDLConcurrentAssign(MEMOBJ_RDEN(memObj), '|');
  HDLConcurrentAssign *wren_assignment = new HDLConcurrentAssign(MEMOBJ_WREN(memObj), '|');
  
  
  //Writing Load 
  for(auto instr_i = memObj.memInstrList.begin(), instr_end = memObj.memInstrList.end(); instr_i!=instr_end; ++instr_i)
  {
    int schedule_time = floor(VIM[static_cast<Value*>(*instr_i)].getScheduledTime());
    int valid_time    = floor(VIM[static_cast<Value*>(*instr_i)].getValidTime());
    String activ_tag = CYCLE_TAG(schedule_time);
    String valid_tag = CYCLE_TAG(valid_time);
    
    switch((*instr_i)->getOpcode())
    {
      case llvm::Instruction::Load:
        {
          HDLInstance *load_op_instance = new HDLInstance( String("loadOp"), String("I" + llvm::utohexstr(reinterpret_cast<uintptr_t>(*instr_i))) ); 
          load_op_instance->setParam((*instr_i)->getType()->getIntegerBitWidth());
          load_op_instance->setParam(1);
          load_op_instance->setPort(String(FUNC_CLK_NET));
          HDLIdentifier activeCycle = getValueHdlName((*instr_i)->getParent()) + activ_tag;
          load_op_instance->setPort(activeCycle);
          load_op_instance->setPort(MEMOBJ_RDATA(memObj));
          HDLIdentifier outputVar = (*instr_i)->getName().str() + valid_tag;
          load_op_instance->setPort(outputVar);
          
          memoryBlock.addStatement(load_op_instance);

          rden_assignment->setRHS(activeCycle);

          HDLIdentifier activeAddr      = getValueHdlName((*instr_i)->getOperand(0)) + activ_tag;
          HDLIdentifier raddr_mux_input =  VERILOG_SIGNAL_CONCAT(activeCycle, activeAddr);
          load_raddr_instance->setPort(raddr_mux_input);

          ld_cnt++;
          break;
        }
      case llvm::Instruction::Store:
        {
          HDLIdentifier activeCycle = getValueHdlName((*instr_i)->getParent())   + activ_tag;
          HDLIdentifier activeAddr  = getValueHdlName((*instr_i)->getOperand(1)) + activ_tag;
          HDLIdentifier activeData  = getValueHdlName((*instr_i)->getOperand(0)) + activ_tag;
          HDLIdentifier waddr_mux_input = VERILOG_SIGNAL_CONCAT(activeCycle, activeAddr);
          store_waddr_instance->setPort(waddr_mux_input);
          HDLIdentifier wdat_mux_input  = VERILOG_SIGNAL_CONCAT(activeCycle, activeData);
          store_wdata_instance->setPort(wdat_mux_input);
          wren_assignment->setRHS(activeCycle);
          st_cnt++;
          break;
        }
      case llvm::Instruction::GetElementPtr:
        {
          //Very naive handling of GEP
          HDLIdentifier gep_output = getValueHdlName(*instr_i) + activ_tag;
          HDLIdentifier gep_input  = getValueHdlName((*instr_i)->getOperand(1)) + activ_tag;
          auto * gep_assignment = new HDLConcurrentAssign(gep_output, '+');
          gep_assignment->setRHS(gep_input);
          for(unsigned i = 2; i < (*instr_i)->getNumOperands(); i++)
          {
            HDLIdentifier gep_input = getValueHdlName((*instr_i)->getOperand(i)) + activ_tag;
            gep_assignment->setRHS(gep_input);
          }
          memoryBlock.addStatement(gep_assignment);
          break;
        }
      default: break;
    }
    
  }

  if (ld_cnt > 0) {
    load_raddr_instance->setParam(ld_cnt);
    load_raddr_instance->setParam(ptrSize);
    load_raddr_instance->setParam(memObj.property.arraylength);
    load_raddr_instance->setPortOthers(0, 16 - ld_cnt);
    memoryBlock.addStatement(load_raddr_instance);
  }

  if (st_cnt > 0) {
    //Write Address mux
    store_waddr_instance->setParam(st_cnt);
    store_waddr_instance->setParam(ptrSize);
    store_waddr_instance->setParam(memObj.property.arraylength);
    store_waddr_instance->setPortOthers(0, 16 - st_cnt);
    memoryBlock.addStatement(store_waddr_instance);

    store_wdata_instance->setParam(st_cnt);
    store_wdata_instance->setParam(memObj.property.bitwidth);
    store_wdata_instance->setParam(memObj.property.bitwidth);
    store_wdata_instance->setPortOthers(0, 16 - st_cnt);
    memoryBlock.addStatement(store_wdata_instance);

  }

  if (ld_cnt > 0)
    memoryBlock.addStatement(rden_assignment);

  if (st_cnt > 0)
    memoryBlock.addStatement(wren_assignment);

  //Write memory 
  if (memObj.property.stype == HdlSignalType::regType)
  {
    HDLInstance *memory_instance = new HDLInstance(String("Memory"), String("memory_" + memObj.name));
    memory_instance->setParam(memObj.property.arraylength);
    memory_instance->setParam(memObj.property.bitwidth);
    memory_instance->setPort(FUNC_CLK_NET);
    memory_instance->setPort(MEMOBJ_WREN(memObj));
    memory_instance->setPort(MEMOBJ_WADDR(memObj));
    memory_instance->setPort(MEMOBJ_WDATA(memObj));
    memory_instance->setPort(MEMOBJ_RDEN(memObj));
    memory_instance->setPort(MEMOBJ_RADDR(memObj));
    memory_instance->setPort(MEMOBJ_RDATA(memObj));

    memoryBlock.addStatement(memory_instance);
  }

  return memoryBlock.generateCode(this);
}

String VerilogGenerator::writeHdlObjDeclaration(HdlObject& obj, String tag)
{   
    LOG_S(VG_DBG+1) << " Writing declaration : " << obj.name << "\n";
    String decl("None");    
    String end_decl("");

    decl = (obj.property.stype == HdlSignalType::inputType)? VERILOG_INPUT :(
              (obj.property.stype == HdlSignalType::outputType)? VERILOG_OUTPUT :""
            );

    end_decl = (obj.property.stype == HdlSignalType::regType)?";\n":",\n";

    String dflt;
    switch (obj.property.vtype)
    {
      case HdlVectorType::scalarType :  if (obj.property.isConstant) 
                                          if (obj.property.dflt >= 0)
                                            dflt += " = " + std::to_string(obj.property.bitwidth) + "'d" + std::to_string(obj.property.dflt);
                                          else 
                                            dflt += " = -" + std::to_string(obj.property.bitwidth) + "'d" + std::to_string(-obj.property.dflt);
                                        
                                        if (obj.property.bitwidth > 1) 
                                          decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, (obj.name + tag + dflt));
                                        else 
                                         decl += VERILOG_VAR_DECL(bit, (obj.name + tag + dflt));
                                        
                                        break;
      case HdlVectorType::memoryType :  decl = "//Memory\n" ; 
                                        if (obj.property.stype != HdlSignalType::regType) {
                                          decl += VERILOG_OUTPUT + VERILOG_VEC_DECL(bit, obj.property.arraylength, MEMOBJ_RADDR(obj)) + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VEC_DECL(bit, obj.property.arraylength, MEMOBJ_WADDR(obj)) + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VEC_DECL(bit, obj.property.bitwidth, MEMOBJ_WDATA(obj)) + end_decl;
                                          decl += VERILOG_INPUT  + VERILOG_VEC_DECL(bit, obj.property.bitwidth, MEMOBJ_RDATA(obj)) + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VAR_DECL(bit, MEMOBJ_WREN(obj)) + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VAR_DECL(bit, MEMOBJ_RDEN(obj));
                                        } else {
                                          decl += VERILOG_VEC_DECL(bit, obj.property.arraylength, MEMOBJ_RADDR(obj)) + end_decl;
                                          decl += VERILOG_VEC_DECL(bit, obj.property.arraylength, MEMOBJ_WADDR(obj)) + end_decl;
                                          decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, MEMOBJ_WDATA(obj)) + end_decl;
                                          decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, MEMOBJ_RDATA(obj)) + end_decl;
                                          decl += VERILOG_VAR_DECL(bit, MEMOBJ_WREN(obj)) + end_decl;
                                          decl += VERILOG_VAR_DECL(bit, MEMOBJ_RDEN(obj));
                                        }
                                        break;
      
      case HdlVectorType::arrayType  :  decl += VERILOG_ARR_DECL(bit, obj.property.bitwidth,  (obj.name + tag) , obj.property.arraylength);
                                        break;
      
      default : break;
    }

    decl += end_decl;

    //If the object is an output array, we attach a valid flag to it 
    if (obj.property.stype == HdlSignalType::outputType && obj.property.vtype == HdlVectorType::arrayType)
    {
      decl += VERILOG_OUTPUT + VERILOG_VEC_DECL(bit, obj.property.arraylength,  (obj.name + "_valid")) +  end_decl;
    }
    
  return decl;
}

