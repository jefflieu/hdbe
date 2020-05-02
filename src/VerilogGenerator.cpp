
#include "CodeGenerator.hpp"
#include "VerilogTemplate.hpp"
#include "logging/logger.hpp"
#include "IRUtil.hpp"

using namespace hdbe;

using String      = std::string;
using Value       = llvm::Value;
using Instruction = llvm::Instruction;

void VerilogGenerator::write() 
{
  auto F         = CDI_h->irFunction;
  String filename = F->getName().str() + ".sv";
  std::ofstream os(filename, std::ofstream::out);  
  
  os << VERILOG_DECL_MODULE(F->getName().str());

  writePorts(os);
  
  writeSignalDeclaration(os);
  
  writeInputAssignment(os);
  
  writeStateSquence(os);
  
  writeInstructions(os);
  
  writeRegisterStages(os);
  
  writeReturnStatement(os);
  
  writeArrayObject(os);
  
  writeVCDLogging(os);

  os << VERILOG_DECL_MODULE_END(F->getName().str());    
  os.close();  
};


String VerilogGenerator::writeHdlObjDeclaration(HdlObject& obj, String tag = "")
{   
    String decl("None");    
    decl = (obj.property.stype == HdlSignalType::inputType)?"input ":(
              (obj.property.stype == HdlSignalType::outputType)?"output ":""
            );
    String dflt;
    switch (obj.property.vtype)
    {
      case HdlVectorType::scalarType : if (obj.property.isConstant) dflt += " = " + std::to_string(obj.property.dflt);
                                       if (obj.property.bitwidth > 1) 
                                         decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, (obj.name + tag + dflt));
                                       else 
                                         decl += VERILOG_VAR_DECL(bit, (obj.name + tag + dflt));
                                       break;
      case HdlVectorType::memoryType : decl += "--Memory port" ; break;
      case HdlVectorType::arrayType  : decl += VERILOG_ARR_DECL(bit, obj.property.bitwidth,  (obj.name + tag) , obj.property.arraylength);
      default : break;
    }
    
  return decl;
}

std::ostream& VerilogGenerator::writeSignalDeclaration(std::ostream& os)
{  
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  auto &memObjList   = CDI_h->memObjList;
  for(auto I = variableList.begin(), E = variableList.end(); I!=E; ++I)
  {
    HdlVariable &var = *I;
    //for each signal, get live time 
    int birthTime = floor(VIM[var.getIrValue()].birthTime.time);
    int liveTime = VIM[var.getIrValue()].getLiveTime() + birthTime;
    LOG_S(0) << *(var.getIrValue()) << "\n";
    for(uint32_t i = birthTime; i <= liveTime; i++)
      {
        String tag = "_" + std::to_string(i);
        os << writeHdlObjDeclaration(var, tag) + ";\n";
      }
  }

  for(auto I = memObjList.begin(), E = memObjList.end(); I!=E; ++I)
  {
    HdlMemory &var = *I;
    //for each signal, get live time
    if (var.property.stype == HdlSignalType::regType)
      os << writeHdlObjDeclaration(var, "") + ";\n";      
  }
}


std::ostream& VerilogGenerator::writePorts(std::ostream& os){
  auto &portList = CDI_h->portList;
  auto F         = CDI_h->irFunction;
  
    
  //Iterate over analyzer.m_portList;  
  for(auto I = portList.begin(), E = portList.end(); I!=E; ++I)
  {
    HdlPort& port = *I;
    os << writeHdlObjDeclaration(port) + ",\n";
  }

  os << VERILOG_HDBE_PORTS;
  return os;
};

std::ostream& VerilogGenerator::writeStateSquence(std::ostream& os){
  
  auto &stateList = CDI_h->stateList;
  String declare;
  String assign;
  String ret_state;
  
  os << VERILOG_CODE_SECTION("State sequence");  
  
  for(auto state_i = stateList.begin(), state_end = stateList.end(); state_i != state_end; ++state_i)
  {    
    HdlState & state = *state_i;
    declare += VERILOG_VAR_DECL(bit, state.getName().str()) + ";\n";                  
    if (state.isBranch()) {
      if (state.isReturn()) {
        
      }
    } else {
      String next_state = makeHdlStateName( String(state.getbbName()), state.id + 1);
      assign += next_state + VERILOG_ASSIGN + state.getName().str() + VERILOG_ENDL;      
    }
  }

  os << declare;
  os << "assign state_entry0 = func_start" << VERILOG_ENDL;
  os << VERILOG_CLKPROCESS_TOP("state_process");
  os << assign;
  os << VERILOG_CLKPROCESS_BOTTOM("state_process");
  
  return os;
};

std::ostream& VerilogGenerator::writeInstructions(std::ostream& os){
  
  std::list<HdlVariable> &variableList = CDI_h->variableList;
  
  os << VERILOG_CODE_SECTION("Instructions");

  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    //
    if (llvm::Instruction::classof(var_i->getIrValue())) {
      LOG(INFO, *(var_i->getIrValue()));
      auto instr = static_cast<Instruction*>(var_i->getIrValue());
      if (isMemoryInstruction(instr)) {
      } else {
      os << writeSimpleInstruction(instr);      
      }
    }
  }  
  
  return os;
};

String VerilogGenerator::writeSimpleInstruction(llvm::Instruction* I)
{
  String instantiate;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  HdlState& state = *(VIM[static_cast<Value*>(I)].birthTime.state);
  if (llvm::BinaryOperator::classof(I)) {    
    size = sprintf(buf,"BinaryOp #( ");
    instantiate += String(buf, size);     
    size = sprintf(buf,"\"%s\",", I->getOpcodeName());
    instantiate += String(buf, size); 
    size = sprintf(buf,"%d)", I->getOperand(0)->getType()->getIntegerBitWidth());
    instantiate += String(buf, size); 
  } else if (llvm::CmpInst::classof(I)) {
    size = sprintf(buf,"%sOp #(", I->getOpcodeName());
    instantiate += String(buf, size);
    size = sprintf(buf,"\"%s\",", llvm::CmpInst::getPredicateName( (static_cast<llvm::CmpInst*>(I))->getPredicate()  ).data());
    instantiate += String(buf, size); 
    size = sprintf(buf,"%d)", I->getType()->getIntegerBitWidth());
    instantiate += String(buf, size);
  } else {  
    size = sprintf(buf,"%sOp #(", I->getOpcodeName());
    instantiate += String(buf, size);
    size = sprintf(buf,"\"%s\",", I->getOpcodeName());
    instantiate += String(buf, size); 
    size = sprintf(buf,"%d)", I->getType()->getIntegerBitWidth());
    instantiate += String(buf, size); 
    //size = sprintf(buf,"#(.BITWIDTH (%d))n", I->getType()->getIntegerBitWidth());
    //instantiate += String(buf, size); 
  }  
  size = sprintf(buf," I%lx ( ", reinterpret_cast<uintptr_t>(I));
  instantiate += String(buf, size);
  size = sprintf(buf,"func_clk, ");
  instantiate += String(buf, size);   
  size = sprintf(buf,"%s, ", state.getName().data());
  instantiate += String(buf, size);
  
  String tag = "_" + std::to_string(state.id);
  for(const llvm::Use &use : I->operands())
    { 
      //size = sprintf(buf,"%s%s, ", (*use).getName().data(), &tag[0]);
      size = sprintf(buf,"%s%s, ", getValueHdlName(use.get()).data(), &tag[0]);
      instantiate += String(buf, size); 
    }

  size = sprintf(buf,"%s%s );\n", I->getName().data(), &tag[0]);  
  instantiate += String(buf, size);   
  return instantiate;  
};

Ostream& VerilogGenerator::writeRegisterStages(Ostream& os)
{
  String assign;
  std::map<llvm::Value*, ValueLifeInfo> &VIM = CDI_h->valueInfoMap;
  std::list<HdlVariable>& variableList       = CDI_h->variableList;

  os << VERILOG_CODE_SECTION("Register stages for pipelining");
  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    if (var_i->property.isConstant) continue;
    int birthCycle = floor(VIM[var_i->getIrValue()].birthTime.time);
    int useCycle   = floor(VIM[var_i->getIrValue()].useTimeList.back().time);
    for(int i = birthCycle + 1; i <= useCycle; ++i)
    {
        String tag1 = "_" + std::to_string(i);
        String tag0 = "_" + std::to_string(i-1);
        assign += var_i->name + tag1 + VERILOG_ASSIGN + var_i->name + tag0 + VERILOG_ENDL;
      
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
        assign += "if (" + state.getName().str() + ")\n";
        assign += "begin\n";
        assign += "func_done <= 1'b1;\n";
        if (state.termInstruction->getNumOperands() > 0)
          assign += "func_ret  <= " + state.termInstruction->getOperand(0)->getName().str() + tag + VERILOG_ENDL;
        assign += "end\n"; 
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
  String str = String(
"\n\n\
initial begin \n\
  if ($test$plusargs(\"trace\") != 0) begin \n\
    $display(\"[%0t] Tracing to logs/vlt_dump.vcd...\", $time); \n\
    $dumpfile(\"logs/vlt_dump.vcd\"); \n\
    $dumpvars(); \n\
  end \n\
  $display(\"[%0t] Model running...\", $time);\n\
end\n\n");
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
    //Writing Load 
    for(auto instr_i = memObj.memInstrList.begin(), instr_end = memObj.memInstrList.end(); instr_i!=instr_end; ++instr_i)
    {
      if ((*instr_i)->getOpcode() == llvm::Instruction::Load){
        HdlState* state = VIM[static_cast<Value*>(*instr_i)].birthTime.state;
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
        String tag0 = "_" + std::to_string(state->id);
        int idx = computeIndex(*instr_i, memObj.getIrValue());
        Value* val = (*instr_i)->getOperand(0);
        store_assign += "if (" + state->name + ")";
        store_assign += "  " + memObj.name + "[" + std::to_string(idx) + "]"+ VERILOG_ASSIGN + val->getName().str() + tag0 + VERILOG_ENDL;
       }
    }
    
  }

  os << load_assign;
  os << VERILOG_CLKPROCESS_TOP("store_handling");
  os << store_assign;
  os << VERILOG_CLKPROCESS_BOTTOM("store_handling");
  return os;
}
