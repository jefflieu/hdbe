
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
    end_decl = (obj.property.stype == HdlSignalType::inputType)?",\n":(
              (obj.property.stype == HdlSignalType::outputType)?",\n":";\n"
            );

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
                                          decl += VERILOG_OUTPUT + VERILOG_VEC_DECL(bit, obj.property.bitwidth, obj.name + "_addr") + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VEC_DECL(bit, obj.property.bitwidth, obj.name + "_wdat") + end_decl;
                                          decl += VERILOG_INPUT  + VERILOG_VEC_DECL(bit, obj.property.bitwidth, obj.name + "_rdat") + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VAR_DECL(bit, obj.name + "_wren") + end_decl;
                                          decl += VERILOG_OUTPUT + VERILOG_VAR_DECL(bit, obj.name + "_rden");
                                        } else {
                                          decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, obj.name + "_addr") + end_decl;
                                          decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, obj.name + "_wdat") + end_decl;
                                          decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, obj.name + "_rdat") + end_decl;
                                          decl += VERILOG_VAR_DECL(bit, obj.name + "_wren") + end_decl;
                                          decl += VERILOG_VAR_DECL(bit, obj.name + "_rden");
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
    int birthTime = floor(VIM[var.getIrValue()].birthTime.time);
    int liveTime = VIM[var.getIrValue()].getLiveTime() + birthTime;
    LOG_S(VG_DBG + 1) << " Timing info " << birthTime << " " << liveTime << "\n";
    
    for(uint32_t i = birthTime; i <= liveTime; i++)
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
    int birthTime = floor(VIM[var.getIrValue()].birthTime.time);
    int useTime   = floor(VIM[var.getDestBB()].useTimeList.back().time);
    LOG_S(VG_DBG + 1) << " Timing info " << birthTime << " " << useTime << "\n";
    
    for(uint32_t i = birthTime; i <= useTime; i++)
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
    int step_id = floor(VIM[static_cast<Value*>(E.getIrValue())].birthTime.time);
    String tag = CYCLE_TAG(step_id);
    statement += VERILOG_ASSIGN_STATEMENT + E.getName().str() + tag + VERILOG_CONT_ASSIGN; 
    statement += writeControlActiveCondition(static_cast<Instruction*>(E.getIrValue()), E.getDestBB(), step_id);
    statement += VERILOG_ENDL;
  }

  statement += String(VERILOG_COMMENT "Basicblock valid conditions\n");

  for(auto bb_iter = F->begin(), bb_end = F->end(); bb_iter != bb_end; bb_iter ++ )
  {
    BasicBlock& bb = *bb_iter;
    HdlState& state = *(VIM[static_cast<Value*>(&bb)].birthTime.state);
    String tag = CYCLE_TAG(state.id);
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
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  HdlState& state = *(VIM[static_cast<Value*>(I)].birthTime.state);
  size = sprintf(buf, "\"%8s\"", I->getOpcodeName());
  String opcodeString = String(buf, size);

  LOG_IF_S(ERROR, ! llvm::PHINode::classof(I)) << "Error, instruction is not PhiNode\n";
  LOG_IF_S(ERROR, ! I->getType()->isIntegerTy()) << "Error, PHINode is not integer type\n";
  assert (llvm::PHINode::classof(I));    
  assert (I->getType()->isIntegerTy());

  auto phi = static_cast<llvm::PHINode*>(I);
  auto N = phi->getNumIncomingValues();

  size = sprintf(buf,"%10sOp #(", "PHINode");
  instantiate += String(buf, size);     
  size = sprintf(buf,"%10d,", static_cast<llvm::PHINode*>(I)->getNumIncomingValues());
  instantiate += String(buf, size); 
  size = sprintf(buf,"%6d)", phi->getType()->getIntegerBitWidth());
  instantiate += String(buf, size);

  size = sprintf(buf," I%lx ( ", reinterpret_cast<uintptr_t>(I));
  instantiate += String(buf, size);
  size = sprintf(buf,"func_clk, ");
  instantiate += String(buf, size);   
  
  String activeCycle = I->getParent()->getName().str() + CYCLE_TAG(state.id);
  size = sprintf(buf,"%s,\n", activeCycle.data());
  instantiate += String(buf, size);
  
  String tag = CYCLE_TAG(state.id);

  size = sprintf(buf,"%s%s%s,\n", space.data(), I->getName().data(), &tag[0]);  
  instantiate += String(buf, size); 
  for(int i = 0; i < 16; i++)
  {
    if (i < N) {
      llvm::BasicBlock* blk = phi->getIncomingBlock(i);
      llvm::Value* val = phi->getIncomingValue(i);
      HdlCFGEdge& edge = CDI_h->findCFGEdge(blk, phi->getParent());
      String edgeName = edge.getName().str();
      String valName = getValueHdlName(val);
      if (!edge.isBackEdge()) {
        valName += tag;
        edgeName += tag;
      } else {
        valName += LOOP_TAG;
        edgeName += LOOP_TAG;
      }

      size = sprintf(buf, "%s{ %-10s, %-20s}", space.data(), edgeName.data(), valName.data());
      instantiate += String(buf, size);
    } else if (i == N) {
      size = sprintf(buf, "%s ", space.data());
      instantiate += String(buf, size);
    } else {
      instantiate += String("0");
    }
    if (i < N) instantiate += ",\n"; else 
    if (i < 15) instantiate += ","; else instantiate += ");\n"; 

  }

  return instantiate;  
} 

String VerilogGenerator::writeSimpleInstruction(llvm::Instruction* I)
{
  String instantiate;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  HdlState& state = *(VIM[static_cast<Value*>(I)].birthTime.state);
  float valid_time = VIM[static_cast<Value*>(I)].birthTime.time;
  size = sprintf(buf, "\"%8s\"", I->getOpcodeName());
  // String opcodeString = "\"" + String(I->getOpcodeName()) + pad + "\"";
  String opcodeString = String(buf, size);
  if (llvm::BinaryOperator::classof(I)) {    
    size = sprintf(buf,"%10sOp #(", "Binary");
    instantiate += String(buf, size);     
    size = sprintf(buf,"%10s,", opcodeString.data() );
    instantiate += String(buf, size); 
    size = sprintf(buf,"%6d)", I->getOperand(0)->getType()->getIntegerBitWidth());
    instantiate += String(buf, size); 
  } else if (llvm::CmpInst::classof(I)) {
    size = sprintf(buf,"%10sOp #(", I->getOpcodeName());
    instantiate += String(buf, size);
    Twine predNameString = "\"" + llvm::CmpInst::getPredicateName( (static_cast<llvm::CmpInst*>(I))->getPredicate()) + "\"";
    size = sprintf(buf,"%10s,", predNameString.str().data());
    instantiate += String(buf, size); 
    size = sprintf(buf,"%6d)", I->getOperand(0)->getType()->getIntegerBitWidth());
    instantiate += String(buf, size);
  } else if (llvm::CastInst::classof(I)) {
    size = sprintf(buf,"%10sOp #(", "Cast");
    instantiate += String(buf, size);
    size = sprintf(buf,"%10s,", opcodeString.data() );
    instantiate += String(buf, size); 
    size = sprintf(buf,"%6d,", I->getOperand(0)->getType()->getIntegerBitWidth());
    instantiate += String(buf, size);
    size = sprintf(buf,"%6d)", I->getType()->getIntegerBitWidth());
    instantiate += String(buf, size);
  } else if (llvm::PHINode::classof(I) || llvm::SwitchInst::classof(I) ||llvm::BranchInst::classof(I)) {
    
  } else {  
    size = sprintf(buf,"%10sOp #(", I->getOpcodeName());
    instantiate += String(buf, size);
    size = sprintf(buf,"%10s,", opcodeString.data() );
    instantiate += String(buf, size); 
    size = sprintf(buf,"%6d)", I->getType()->getIntegerBitWidth());
    instantiate += String(buf, size); 
    //size = sprintf(buf,"#(.BITWIDTH (%d))n", I->getType()->getIntegerBitWidth());
    //instantiate += String(buf, size); 
  }  
  size = sprintf(buf," I%lx ( ", reinterpret_cast<uintptr_t>(I));
  instantiate += String(buf, size); 
  size = sprintf(buf,"func_clk, ");
  instantiate += String(buf, size);   
  
  //size = sprintf(buf,"%s, ", state.getName().data());
  //
  String activeCycle = I->getParent()->getName().str() + CYCLE_TAG(state.id);
  size = sprintf(buf,"%s, ", activeCycle.data());
  instantiate += String(buf, size);
  
  String execute_tag = CYCLE_TAG(state.id);
  for(const llvm::Use &use : I->operands())
    { 
      size = sprintf(buf,"%s%s, ", getValueHdlName(use.get()).data(), &execute_tag[0]);
      instantiate += String(buf, size); 
    }

  String valid_tag = CYCLE_TAG(static_cast<int>(valid_time));
  size = sprintf(buf,"%s%s );\n", I->getName().data(), &valid_tag[0]);  
  instantiate += String(buf, size);   
  return instantiate;  
};

Ostream& VerilogGenerator::writeRegisterStages(Ostream& os)
{
  String assign;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  std::list<HdlVariable>& variableList       = CDI_h->variableList;
  std::list<HdlCFGEdge >& transitionList     = CDI_h->transitionList;

  os << VERILOG_CODE_SECTION("Register stages for pipelining");
  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.isConstant) continue;
    int birthCycle = floor(VIM[var_i->getIrValue()].birthTime.time);
    int useCycle   = floor(VIM[var_i->getIrValue()].useTimeList.back().time);
    for(int i = birthCycle + 1; i <= useCycle; ++i)
    {
        assign += var_i->name + CYCLE_TAG(i) + VERILOG_ASSIGN + var_i->name + CYCLE_TAG(i-1) + VERILOG_ENDL;
    }

    if (var_i->property.isBackValue)
    {
      String tag0 = CYCLE_TAG(birthCycle);
      String activeCycle = static_cast<Instruction*>(var_i->getIrValue())->getParent()->getName().str() + tag0;
      //if (activeCycle) variable_loop <= variable_n;
      assign += VERILOG_IF(activeCycle) + NEW_LINE + 
                  INDENT(1) + var_i->name + LOOP_TAG + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;      
    }
  }



 assign += String(VERILOG_COMMENT) + String("Transitions pipeline\n");
 for(auto var_i = transitionList.begin(), var_last = transitionList.end(); var_i != var_last; ++var_i)
  {
    //for each signal, get live time     
    int birthTime = floor(VIM[var_i->getIrValue()].birthTime.time);
    int useTime   = floor(VIM[var_i->getDestBB()].useTimeList.back().time);    
    for(uint32_t i = birthTime + 1; i <= useTime; i++)
      {
        String tag1 = "_" + std::to_string(i);
        String tag0 = "_" + std::to_string(i-1);
        assign += var_i->name + tag1 + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;
      }
    if (var_i->isBackEdge())
    {
      String tag0 = "_" + std::to_string(birthTime);
      assign += var_i->name + "_loop" + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;
    }
  }

  os << VERILOG_CLKPROCESS_TOP("register_stage");
  os << assign;
  os << VERILOG_CLKPROCESS_BOTTOM("register_stage");
  return os;
}

Ostream& VerilogGenerator::writeInputAssignment(Ostream& os)
{
  String assign;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  std::list<HdlPort>& portList       = CDI_h->portList;

  os <<VERILOG_CODE_SECTION("Copying inputs");
  for(auto var_i = portList.begin(), var_last = portList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.stype != HdlSignalType::inputType) continue;
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
        } else 
          assign += VERILOG_IF(state.getName().str()) + "\n";
        assign += VERILOG_BEGIN;
        assign += "func_done <= 1'b1;\n";
        if (state.termInstruction != nullptr && state.termInstruction->getNumOperands() > 0)
          assign += "func_ret  <= " + state.termInstruction->getOperand(0)->getName().str() + tag + VERILOG_ENDL;
        assign += VERILOG_END; 
      }
    }
  }

  os << VERILOG_CLKPROCESS_TOP("return_handling");
  os << "func_done <= 1'b0;\n";
  os << assign;
  os << VERILOG_CLKPROCESS_BOTTOM("return_handling");
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
  
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  auto &memObjList   = CDI_h->memObjList;
  String load_assign;
  String store_assign;

  os << VERILOG_CODE_SECTION("Array and memory load/store");
  for(auto I = memObjList.begin(), E = memObjList.end(); I!=E; ++I)
  {
    HdlMemory &memObj = *I;
  
    os << writeArrayObject(memObj);
    
  }

  // os << load_assign;
  // os << VERILOG_CLKPROCESS_TOP("store_handling");
  // os << store_assign;
  // os << VERILOG_CLKPROCESS_BOTTOM("store_handling");
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
      //HdlState* state = VIM[static_cast<Value*>(*instr_i)].birthTime.state;
      int birthCycle = floor(VIM[static_cast<Value*>(*instr_i)].birthTime.time);
      String tag0 = "_" + std::to_string(birthCycle);
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
      HdlState* state = VIM[static_cast<Value*>(*instr_i)].birthTime.state;
      String tag0 = CYCLE_TAG(state->id);
      int idx = computeIndex(*instr_i, memObj.getIrValue());
      Value* val = (*instr_i)->getOperand(0);
      store_assign += "if (" + state->name + VERILOG_LOGICAL_AND + (*instr_i)->getParent()->getName().str() + tag0 + ")";
      store_assign += "  " + memObj.name + "[" + std::to_string(idx) + "]"+ VERILOG_ASSIGN + val->getName().str() + tag0 + VERILOG_ENDL;
      if (memObj.property.stype == HdlSignalType::outputType)
        store_assign += "  " + memObj.name + VALID_TAG + "[" + std::to_string(idx) + "]"+ VERILOG_ASSIGN + state->name + VERILOG_ENDL;
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

String VerilogGenerator::writeMemoryObject(HdlMemory &memory) {};
