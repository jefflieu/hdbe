
#include "CodeGenerator.hpp"
#include "VerilogTemplate.hpp"

using namespace hdbe;

using String = std::string;

void VerilogGenerator::write() 
{
  String filename = "Test.sv";
  std::ofstream os(filename, std::ofstream::out);  
  writePorts(os);
  
  writeSignalDeclaration(os);

  writeStateSquence(os);
  
  writeInstructions(os);

  os << VERILOG_DECL_MODULE_END("NONE");    
  os.close();  
};


String VerilogGenerator::writeHdlObjDeclaration(HdlObject& obj, String tag = "")
{   
    String decl("None");    
    decl = (obj.property.stype == HdlSignalType::inputType)?"input ":(
              (obj.property.stype == HdlSignalType::outputType)?"output ":"  "
            );
    switch (obj.property.vtype)
    {
      case HdlVectorType::scalarType : if (obj.property.bitwidth > 1) 
                                         decl += VERILOG_VEC_DECL(bit, obj.property.bitwidth, (obj.name + tag));
                                       else 
                                         decl += VERILOG_VAR_DECL(bit, (obj.name + tag));
                                       break;
      case HdlVectorType::memoryType : decl += "--Memory port" ; break;
      case HdlVectorType::arrayType  : decl += "bit [" + std::to_string(obj.property.bitwidth-1) + ":0] " + obj.name + tag + "[" + std::to_string(obj.property.arraylength-1) + ":0]";
      default : break;
    }
    
  return decl;
}

std::ostream& VerilogGenerator::writeSignalDeclaration(std::ostream& os)
{  
  auto &variableList = CDI_h->variableList;
  auto &VIM          = CDI_h->valueInfoMap;
  for(auto I = variableList.begin(), E = variableList.end(); I!=E; ++I)
  {
    HdlVariable &var = *I;
    //for each signal, get live time 
    int birthTime = floor(VIM[var.getIrValue()].birthTime.time);
    int liveTime = VIM[var.getIrValue()].getLiveTime() + birthTime;
    for(uint32_t i = birthTime; i <= liveTime; i++)
      {
        String tag = "_" + std::to_string(i);
        os << writeHdlObjDeclaration(var, tag) + ";\n";
      }
  }
}


std::ostream& VerilogGenerator::writePorts(std::ostream& os){
  auto &portList = CDI_h->portList;
  auto F         = CDI_h->irFunction;
  
  os << VERILOG_DECL_MODULE(F->getName().str());
    
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

  os << " // State sequence " << "\n\n";  
  
  for(auto state_i = stateList.begin(), state_end = stateList.end(); state_i != state_end; ++state_i)
  {    
    HdlState & state = *state_i;
    declare += VERILOG_VAR_DECL(bit, state.getName().str()) + ";\n";                  
    if (state.isBranch()) {
      if (state.isReturn()) {
        ret_state = state.getName();
      }
    } else {
      String next_state = makeHdlStateName( String(state.getbbName()), state.id + 1);
      assign += next_state + VERILOG_ASSIGN + state.getName().str() + VERILOG_ENDL;      
    }
  }

  os << declare;
  os << "state_entry0 <= func_start" << VERILOG_ENDL;
  os << VERILOG_CLKPROCESS_TOP(state_process);
  os << assign;
  os << "func_done <= " + ret_state + VERILOG_ENDL;
  os << VERILOG_CLKPROCESS_BOTTOM(state_process);
  
  return os;
};

std::ostream& VerilogGenerator::writeInstructions(std::ostream& os){
  /*
  std::list<ControlStep> &statelist = scheduler->m_ctrlSteps;
  
  os << " // Instructions " << "\n\n";  
  for(auto cs = statelist.begin(); cs!=statelist.end(); cs++)
  {    
    ControlStep& s = *cs;
    os << " // " << cs->getbbName() << cs->getId() << "\n";    
    for(auto I = s.m_instrList.begin(), E = s.m_instrList.end(); I!=E; ++I)
      {
        os << writeOneInstruction(*I, *cs);
      }
  }  
  */
  return os;
};

String VerilogGenerator::writeOneInstruction(const llvm::Instruction* I)
{
  String executor;
  /*
  std::list<ControlStep> &statelist = scheduler->m_ctrlSteps;
  std::map<const llvm::Value*, ValueLifeInfo> &info = scheduler->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  String state = makeHdlStateName(s.getbbName(), s.getId());
  
  if (I->isBinaryOp()) {    
    size = sprintf(buf,"binary_unit #(\n");
    executor += String(buf, size);     
    size = sprintf(buf,".BITWIDTH (%d),\n", I->getOperand(0)->getType()->getIntegerBitWidth());
    executor += String(buf, size); 
    size = sprintf(buf,".OPCODE  (\"%s\"))\n", I->getOpcodeName());
    executor += String(buf, size); 
  } else {  
    size = sprintf(buf,"%s_unit ", I->getOpcodeName());
    executor += String(buf, size);
    //size = sprintf(buf,"#(.BITWIDTH (%d))n", I->getType()->getIntegerBitWidth());
    //executor += String(buf, size); 
  }  
  size = sprintf(buf,"I%lx(\n", reinterpret_cast<uintptr_t>(I));
  executor += String(buf, size);
  for(const llvm::Use &use : I->operands())
    { 
      size = sprintf(buf,".operand[%d] (%20s),\n", use.getOperandNo(), (*use).getName().data());
      executor += String(buf, size); 
    }
  size = sprintf(buf,".%-10s (%20s),\n","enable", &state[0]);
  executor += String(buf, size);
  size = sprintf(buf,".%-10s (%20s),\n","clk", "func_clk");
  executor += String(buf, size);   
  size = sprintf(buf,".%-10s (%20s));\n\n","result", I->getName().data());  
  executor += String(buf, size);   
  */
  return executor;  
};
