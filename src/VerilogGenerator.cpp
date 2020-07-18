
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
  
  writeStateSquence(os);

  writeCtrlFlow(os);
  
  writeInstructions(os);
  
  writeRegisterStages(os);
  
  writeReturnStatement(os);
  
  writeArrayObject(os);
  
  writeVCDLogging(os);

  os << VERILOG_DECL_MODULE_END(F->getName().str());    
  os.close();  
};


/// Tagging of a variable: 
/// Each HDL variable corresponds to one or many value in IR
/// In order to pipeline the dataflow, we introduce "tag"s to variable 
/// A variable has _n tag which indicates the clock cycle in which it's valid 
/// For example, an instruction is executed at clock cycle 3: a <= b + c 
/// The instruction is described as following in HDL: 
///   a_3 <= b_3 + c_3;
/// This tagging mechanism helps us to pipeline the dataflow and invoke the function repeatedly every clock cycle
String VerilogGenerator::writeHdlObjDeclaration(HdlObject& obj, String tag = "")
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

std::ostream& VerilogGenerator::writeSignalDeclaration(std::ostream& os)
{  
  auto &variableList    = CDI_h->variableList;
  auto &VIM             = CDI_h->valueInfoMap;
  auto &memObjList      = CDI_h->memObjList;
  auto &transitionList  = CDI_h->transitionList;
  LOG_START(VG_DBG);

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
        os << writeHdlObjDeclaration(var, tag);
      }
    
    if (var.property.isBackValue)
    {
      os << writeHdlObjDeclaration(var, LOOP_TAG);
    }

  }

  //Memory objects list including array type 
  for(auto I = memObjList.begin(), E = memObjList.end(); I!=E; ++I)
  {
    HdlMemory &var = *I;
    //for each signal, get live time
    if (var.property.stype == HdlSignalType::regType)
      os << writeHdlObjDeclaration(var, "");      
  }

  //State transition list 
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
      os << writeHdlObjDeclaration(var, tag);
    }
    if (var.isBackEdge())
    {
      os << writeHdlObjDeclaration(var, LOOP_TAG);      
    } 
  }
}


std::ostream& VerilogGenerator::writePorts(std::ostream& os){
  auto &portList = CDI_h->portList;
  auto F         = CDI_h->irFunction;
  
  LOG_START(VG_DBG);
  
  //Iterate over analyzer.m_portList;  
  for(auto I = portList.begin(), E = portList.end(); I!=E; ++I)
  {
    HdlPort& port = *I;
    os << writeHdlObjDeclaration(port);
  }

  os << VERILOG_HDBE_PORTS;
  return os;
};

std::ostream& VerilogGenerator::writeStateSquence(std::ostream& os){
  
  auto &stateList = CDI_h->stateList;
  String declare;
  String assign;
  String ret_state;
  
  LOG_START(VG_DBG);

  os << VERILOG_CODE_SECTION("State sequence");  
  
  for(auto state_i = stateList.begin(), state_end = stateList.end(); state_i != state_end; ++state_i)
  {    
    HdlState & state = *state_i;
    declare += VERILOG_VAR_DECL(bit, state.getName().str()) + ";\n";                  
    if (state.isBranch()) {
      if (state.isReturn()) {
        
      }
    } else {
      String next_state = makeHdlStateName(state.id + 1);
      assign += next_state + VERILOG_ASSIGN + state.getName().str() + VERILOG_ENDL;      
    }
  }

  os << declare;
  os << VERILOG_ASSIGN_STATEMENT << stateList.front().getName().str() << VERILOG_CONT_ASSIGN << "func_start" << VERILOG_ENDL;
  os << VERILOG_CLKPROCESS_TOP("state_process");
  os << assign;
  os << VERILOG_CLKPROCESS_BOTTOM("state_process");
  
  LOG_DONE(VG_DBG);

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
  
  os << VERILOG_CODE_SECTION("Control flow");
  statement = String(VERILOG_COMMENT "//Transition conditions\n");  
  for(auto &E : TL)
  {
    //For each transition edge we write the valid condition of it 
    int step_id = floor(VIM[static_cast<Value*>(E.getIrValue())].valid.time);
    String tag = CYCLE_TAG(step_id);
    statement += VERILOG_ASSIGN_STATEMENT + E.getName().str() + tag + VERILOG_CONT_ASSIGN; 
    statement += writeControlActiveCondition(static_cast<Instruction*>(E.getIrValue()), E.getDestBB(), step_id);
    statement += VERILOG_ENDL;
  }

  statement += String(VERILOG_COMMENT "Basicblock valid conditions\n");

  for(auto bb_iter = F->begin(), bb_end = F->end(); bb_iter != bb_end; bb_iter ++ )
  {
    BasicBlock& bb = *bb_iter;
    int schedule_time = floor(VIM[static_cast<Value*>(&bb)].getScheduledTime());
    String tag = CYCLE_TAG(schedule_time);
    statement += VERILOG_ASSIGN_STATEMENT + bb.getName().str() + tag + VERILOG_CONT_ASSIGN; 
    if (bb.hasNPredecessors(0))
    {
      statement += String("func_start") + String(VERILOG_ENDL);
    } else {
      int cnt = 0;
      for(auto E : TL)
      {
        if (E.getDestBB() == &bb)
        {
           String backedgeTag = LOOP_TAG;
           statement += ((cnt == 0)?" ":"|");
           statement += E.getName().str() + (E.isBackEdge()?backedgeTag:tag);
           cnt ++;
        }
      }
      statement += VERILOG_ENDL;
    }

  }
  os << statement;

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
      loop_assign += INDENT(1) + VERILOG_ASSIGN_STATEMENT + valName + VERILOG_CONT_ASSIGN + getValueHdlName(val) + CYCLE_TAG(edgeValidTime + 1) + VERILOG_ENDL;
    }
    String port = "{" + edgeName + "," + valName + "}";
    phiInstance.setPort(port);
  }
  phiInstance.setPortOthers(0, 16 - N);
 
  instantiate  = phiInstance.getInstanceString(this);
  instantiate += loop_assign; 
  return instantiate;  
} 

String VerilogGenerator::writeSimpleInstruction(llvm::Instruction* I)
{
  HDLInstance *instance;
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
    instance = new HDLInstance("BinaryOp", reinterpret_cast<uintptr_t>(I));
    instance->setParam(opcodeString);
    instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());

  } else if (llvm::CmpInst::classof(I)) {
    instance = new HDLInstance( ( String(I->getOpcodeName()) + "Op"), reinterpret_cast<uintptr_t>(I));
    Twine predNameString = "\"" + llvm::CmpInst::getPredicateName( (static_cast<llvm::CmpInst*>(I))->getPredicate()) + "\"";
    instance->setParam(predNameString.str());
    instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());

  } else if (llvm::CastInst::classof(I)) {
    instance = new HDLInstance( ("CastOp"), reinterpret_cast<uintptr_t>(I));
    instance->setParam(opcodeString);
    instance->setParam(I->getOperand(0)->getType()->getIntegerBitWidth());
    instance->setParam(I->getType()->getIntegerBitWidth());
  
  } else if (llvm::PHINode::classof(I) || llvm::SwitchInst::classof(I) ||llvm::BranchInst::classof(I)) {
     return String("/* empty */");
  
  } else {  
    instance = new HDLInstance( (String(I->getOpcodeName()) + "Op"), reinterpret_cast<uintptr_t>(I));
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

  return instance->getInstanceString(this);  
};

Ostream& VerilogGenerator::writeRegisterStages(Ostream& os)
{
  String assign;
  String loop_variable_assign;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  std::list<HdlVariable>& variableList       = CDI_h->variableList;
  std::list<HdlCFGEdge >& transitionList     = CDI_h->transitionList;

  os << VERILOG_CODE_SECTION("Register stages for pipelining");
  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.isConstant) continue;
    int valid_time = floor(VIM[var_i->getIrValue()].getValidTime());
    int use_time   = floor(VIM[var_i->getIrValue()].getLatestUseTime());
    for(int i = valid_time + 1; i <= use_time; ++i)
    {
        assign += var_i->name + CYCLE_TAG(i) + VERILOG_ASSIGN + var_i->name + CYCLE_TAG(i-1) + VERILOG_ENDL;
    }

    if (var_i->property.isBackValue)
    {
      String tag0 = CYCLE_TAG(valid_time);
      String activeCycle = static_cast<Instruction*>(var_i->getIrValue())->getParent()->getName().str() + tag0;
      //if (activeCycle) variable_loop <= variable_n;
      //assign += VERILOG_IF(activeCycle) + NEW_LINE + 
      //            INDENT(1) + var_i->name + LOOP_TAG + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;      
      //loop_variable_assign += VERILOG_ASSIGN_STATEMENT + var_i->name + LOOP_TAG + VERILOG_CONT_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;
    }
  }



 assign += String(VERILOG_COMMENT) + String("Transitions pipeline\n");
 for(auto var_i = transitionList.begin(), var_last = transitionList.end(); var_i != var_last; ++var_i)
  {
    //for each signal, get live time     
    int valid_time  = floor(VIM[var_i->getIrValue()].getValidTime());
    int use_time    = floor(VIM[var_i->getDestBB()].getLatestUseTime());    
    for(uint32_t i = valid_time + 1; i <= use_time; i++)
      {
        String tag1 = "_" + std::to_string(i);
        String tag0 = "_" + std::to_string(i-1);
        assign += var_i->name + tag1 + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;
      }
    if (var_i->isBackEdge())
    {
      String tag0 = "_" + std::to_string(valid_time);
      assign += var_i->name + LOOP_TAG + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;
    }
  }

  os << loop_variable_assign;
  os << VERILOG_CLKPROCESS_TOP("register_stage");
  os << assign;
  os << VERILOG_CLKPROCESS_BOTTOM("register_stage");
  return os;
}

Ostream& VerilogGenerator::writeInputAssignment(Ostream& os)
{
  String assign;
  std::list<HdlPort>& portList       = CDI_h->portList;

  os <<VERILOG_CODE_SECTION("Copying inputs");
  for(auto var_i = portList.begin(), var_last = portList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.stype != HdlSignalType::inputType) continue;
    if (var_i->property.vtype == HdlVectorType::memoryType) continue;
    if (var_i->property.isUnused) continue;
    String tag0 = "_0";
    assign += VERILOG_ASSIGN_STATEMENT + var_i->name + tag0 + VERILOG_CONT_ASSIGN + var_i->name + VERILOG_ENDL;
  } 
  os << assign;
  return os;
}

Ostream& VerilogGenerator::writeReturnStatement(Ostream& os)
{
  auto &stateList = CDI_h->stateList;
  String assign;
  String comb_assign;
  bool ret_void = false;
  os << VERILOG_CODE_SECTION("Return handling");    
  for(auto state_i = stateList.begin(), state_end = stateList.end(); state_i != state_end; ++state_i)
  {    
    HdlState & state = *state_i;
    if (state.isBranch()) {
      if (state.isReturn()) {
        String tag = "_" + std::to_string(state.id);
        if (state.termInstruction != nullptr) {
          String bbName = state.termInstruction->getParent()->getName().str() + tag;
          assign += VERILOG_IF(bbName) + "\n";
          comb_assign += VERILOG_ASSIGN_STATEMENT + String("func_done") + VERILOG_CONT_ASSIGN + bbName + VERILOG_ENDL;
        } else {
          assign += VERILOG_IF(state.getName().str()) + "\n";
          comb_assign += VERILOG_ASSIGN_STATEMENT + String("func_done") + VERILOG_CONT_ASSIGN + state.getName().str() + VERILOG_ENDL;
        }
        assign += VERILOG_BEGIN;
        assign += "func_done <= 1'b1;\n";
        if (state.termInstruction != nullptr && state.termInstruction->getNumOperands() > 0) {
          assign += "func_ret  <= " + state.termInstruction->getOperand(0)->getName().str() + tag + VERILOG_ENDL;
          comb_assign += VERILOG_ASSIGN_STATEMENT + String("func_ret") + VERILOG_CONT_ASSIGN + state.termInstruction->getOperand(0)->getName().str() + tag + VERILOG_ENDL;
        } else {
          ret_void = true;
        }
        assign += VERILOG_END; 
      }
    }
  }

  if (false  && (!ret_void)) {
    //Register stage at the return function
    os << VERILOG_CLKPROCESS_TOP("return_handling");
    os << "func_done <= 1'b0;\n";
    os << assign;
    os << VERILOG_CLKPROCESS_BOTTOM("return_handling");
  } else {
    //No register at the return
    os << comb_assign;
  }
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
  String load_assign;
  String store_assign;

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
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  String load_assign;
  String store_assign;
  //Writing Load 
  for(auto instr_i = memObj.memInstrList.begin(), instr_end = memObj.memInstrList.end(); instr_i!=instr_end; ++instr_i)
  {
    if ((*instr_i)->getOpcode() == llvm::Instruction::Load){
      int valid_time = floor(VIM[static_cast<Value*>(*instr_i)].getValidTime());
      String tag0 = "_" + std::to_string(valid_time);
      int idx = computeIndex(*instr_i, memObj.getIrValue());
      if (memObj.property.stype == HdlSignalType::inputType)
        load_assign += VERILOG_ASSIGN_STATEMENT + (*instr_i)->getName().str() + tag0 + VERILOG_CONT_ASSIGN + memObj.name + tag0 + "[" + std::to_string(idx) + "]"+ VERILOG_ENDL;
      else 
        load_assign += VERILOG_ASSIGN_STATEMENT + (*instr_i)->getName().str() + tag0 + VERILOG_CONT_ASSIGN + memObj.name + "[" + std::to_string(idx) + "]"+ VERILOG_ENDL;
    }
  }   
  
  for(auto instr_i = memObj.memInstrList.begin(), instr_end = memObj.memInstrList.end(); instr_i!=instr_end; ++instr_i)
  {
    if ((*instr_i)->getOpcode() == llvm::Instruction::Store){
      int schedule_time = floor(VIM[static_cast<Value*>(*instr_i)].getScheduledTime());
      String tag0 = CYCLE_TAG(schedule_time);
      int idx = computeIndex(*instr_i, memObj.getIrValue());
      Value* val = (*instr_i)->getOperand(0);
      String execute_state = (*instr_i)->getParent()->getName().str() + tag0;
      store_assign += "if (" + execute_state + ")";
      store_assign += "  " + memObj.name + "[" + std::to_string(idx) + "]"+ VERILOG_ASSIGN + val->getName().str() + tag0 + VERILOG_ENDL;
      if (memObj.property.stype == HdlSignalType::outputType)
        store_assign += "  " + memObj.name + VALID_TAG + "[" + std::to_string(idx) + "]"+ VERILOG_ASSIGN + execute_state + VERILOG_ENDL;
      }
  }

  String ret = String("\n\n/*") + memObj.name + String("*/\n");
  ret += String("//Load handling\n");
  ret += load_assign;
  ret += String(VERILOG_CLKPROCESS_TOP("store_handling"));
  ret += store_assign;
  ret += String(VERILOG_CLKPROCESS_BOTTOM("store_handling"));  
  return ret;
}

String VerilogGenerator::writeMemoryObject(HdlMemory &memObj)
{
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  unsigned ptrSize   = CDI_h->getPointerSizeInBits();
  String GEP;
  String load_assign;
  String load_op_instantiate;
  String rden_assign;
  String wren_assign;
  unsigned ld_cnt = 0;
  unsigned st_cnt = 0;
  char buf[256];
  unsigned size = 0;

  HDLInstance load_raddr_instance( String("DataMux"), String("RAddr_of_" + memObj.name) ); 
  load_raddr_instance.setPort(String(FUNC_CLK_NET));
  load_raddr_instance.setPort(String(MEMOBJ_RADDR(memObj)));
  
  HDLInstance store_waddr_instance( String("DataMux"), String("WAddr_of_" + memObj.name) ); 
  store_waddr_instance.setPort(String(FUNC_CLK_NET));
  store_waddr_instance.setPort(String(MEMOBJ_WADDR(memObj)));
  
  HDLInstance store_wdata_instance( String("DataMux"), String("WData_of_" + memObj.name) ); 
  store_wdata_instance.setPort(String(FUNC_CLK_NET));
  store_wdata_instance.setPort(String(MEMOBJ_WDATA(memObj)));
  
  //Writing Load 
  for(auto instr_i = memObj.memInstrList.begin(), instr_end = memObj.memInstrList.end(); instr_i!=instr_end; ++instr_i)
  {
    int schedule_time = floor(VIM[static_cast<Value*>(*instr_i)].getScheduledTime());
    int valid_time    = floor(VIM[static_cast<Value*>(*instr_i)].getValidTime());
    String tag0 = CYCLE_TAG(schedule_time);
    String valid_tag = CYCLE_TAG(valid_time);
    
    switch((*instr_i)->getOpcode())
    {
      case llvm::Instruction::Load:
          {
          HDLInstance load_op_instance( String("loadOp"), String("I" + llvm::utohexstr(reinterpret_cast<uintptr_t>(*instr_i))) ); 
          load_op_instance.setParam((*instr_i)->getType()->getIntegerBitWidth());
          load_op_instance.setParam(1);
          load_op_instance.setPort(String(FUNC_CLK_NET));
          String activeCycle = getValueHdlName((*instr_i)->getParent()) + tag0;
          load_op_instance.setPort(activeCycle);
          load_op_instance.setPort(MEMOBJ_RDATA(memObj));
          String outputVar = (*instr_i)->getName().str() + valid_tag;
          load_op_instance.setPort(outputVar);

          load_op_instantiate += load_op_instance.getInstanceString(this); 

          rden_assign  += String((ld_cnt > 0)?"|":"") + getValueHdlName((*instr_i)->getParent()) + tag0;
          
          String raddr_mux_input =  "{" + getValueHdlName((*instr_i)->getParent()) + tag0 + ", " + getValueHdlName((*instr_i)->getOperand(0)) + tag0 + "}";
          load_raddr_instance.setPort(raddr_mux_input);

          ld_cnt++;
          break;
          }
      case llvm::Instruction::Store:
          {
          String waddr_mux_input = "{" + getValueHdlName((*instr_i)->getParent()) + tag0 + ", " + getValueHdlName((*instr_i)->getOperand(1)) + tag0 + "}";
          store_waddr_instance.setPort(waddr_mux_input);
          String wdat_mux_input = "{" + getValueHdlName((*instr_i)->getParent()) + tag0 + ", " + getValueHdlName((*instr_i)->getOperand(0)) + tag0 + "}";
          store_wdata_instance.setPort(wdat_mux_input);

          wren_assign  += String((st_cnt > 0)?"|":"") + getValueHdlName((*instr_i)->getParent()) + tag0;
          st_cnt++;
          break;
          }
      case llvm::Instruction::GetElementPtr:
          //Very naive handling of GEP
          GEP += VERILOG_ASSIGN_STATEMENT + (*instr_i)->getName().str() + tag0 + VERILOG_CONT_ASSIGN + getValueHdlName((*instr_i)->getOperand(1)) + tag0;
          for(unsigned i = 2; i < (*instr_i)->getNumOperands(); i++)
          {
            GEP += "+" + getValueHdlName((*instr_i)->getOperand(i)) + tag0;
          }
          GEP += VERILOG_ENDL;
          break;
      default: break;
    }
    
  }

  String instantiate;
  instantiate += GEP;
  instantiate += load_op_instantiate;
  if (ld_cnt > 0) {
    load_raddr_instance.setParam(ld_cnt);
    load_raddr_instance.setParam(ptrSize);
    load_raddr_instance.setParam(memObj.property.arraylength);
    load_raddr_instance.setPortOthers(0, 16 - ld_cnt);
    instantiate += load_raddr_instance.getInstanceString(this);

    
  }

  if (st_cnt > 0) {
    //Write Address mux
    store_waddr_instance.setParam(st_cnt);
    store_waddr_instance.setParam(ptrSize);
    store_waddr_instance.setParam(memObj.property.arraylength);
    store_waddr_instance.setPortOthers(0, 16 - st_cnt);
    instantiate += store_waddr_instance.getInstanceString(this);

    store_wdata_instance.setParam(st_cnt);
    store_wdata_instance.setParam(memObj.property.bitwidth);
    store_wdata_instance.setParam(memObj.property.bitwidth);
    store_wdata_instance.setPortOthers(0, 16 - st_cnt);
    instantiate += store_wdata_instance.getInstanceString(this);

    
  }

  if (ld_cnt > 0)
    instantiate += VERILOG_ASSIGN_STATEMENT + MEMOBJ_RDEN(memObj) + VERILOG_CONT_ASSIGN + rden_assign + VERILOG_ENDL;
  if (st_cnt > 0)
    //Store assignment 
    instantiate += VERILOG_ASSIGN_STATEMENT + MEMOBJ_WREN(memObj) + VERILOG_CONT_ASSIGN + wren_assign + VERILOG_ENDL;

  //Write memory 
  if (memObj.property.stype == HdlSignalType::regType)
  {
    HDLInstance memory_instance( String("Memory"), String("memory_" + memObj.name));
    memory_instance.setParam(memObj.property.arraylength);
    memory_instance.setParam(memObj.property.bitwidth);
    memory_instance.setPort(FUNC_CLK_NET);
    memory_instance.setPort(MEMOBJ_WREN(memObj));
    memory_instance.setPort(MEMOBJ_WADDR(memObj));
    memory_instance.setPort(MEMOBJ_WDATA(memObj));
    memory_instance.setPort(MEMOBJ_RDEN(memObj));
    memory_instance.setPort(MEMOBJ_RADDR(memObj));
    memory_instance.setPort(MEMOBJ_RDATA(memObj));

    instantiate += memory_instance.getInstanceString(this);
  }

  String ret = VERILOG_CODE_SECTION(memObj.name);
  ret += instantiate;
  return ret;
}


