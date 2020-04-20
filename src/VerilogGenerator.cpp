
#include "CodeGenerator.hpp"
#include "VerilogTemplate.hpp"

using namespace hdbe;

void VerilogGenerator::write() 
{
  std::string filename = "Test.sv";
  std::ofstream os(filename, std::ofstream::out);  
  writePorts(os);
  
  writeSignalDeclaration(os);

  writeStateSquence(os);
  
  writeInstructions(os);

  os << VERILOG_DECL_MODULE_END("NONE");    
  os.close();  
};


std::string VerilogGenerator::writeHdlObjDeclaration(HdlObject& obj)
{   
    std::string decl("None");    
    //std::map<const llvm::Value*, ValueLifeInfo> &info = scheduler->valueInfoMap;    
    //unsigned regStageNum = info[obj.m_irValue].getRegisterStage();
    decl = (obj.property.stype == HdlSignalType::inputType)?"input ":(
              (obj.property.stype == HdlSignalType::outputType)?"output ":"  "
            );
    switch (obj.property.vtype)
    {
      case HdlVectorType::scalarType : if (obj.property.bitwidth > 1) 
                                         decl += "bit [" + std::to_string(obj.property.bitwidth-1) + ":0] " + obj.name;
                                       else 
                                         decl += "bit " + obj.name;
                                       break;
      case HdlVectorType::memoryType : decl += "--Memory port" ; break;
      case HdlVectorType::arrayType  : decl += "bit [" + std::to_string(obj.property.bitwidth-1) + ":0] " + obj.name + "[" + std::to_string(obj.property.arraylength-1) + ":0]";
      default : break;
    }
    //decl += "//" + std::to_string(regStageNum);
    
  return decl;
}

std::ostream& VerilogGenerator::writeSignalDeclaration(std::ostream& os)
{  
  auto &variableList = CDI_h->variableList;
  
  for(auto I = variableList.begin(), E = variableList.end(); I!=E; ++I)
  {
    HdlVariable &var = *I;
    os << writeHdlObjDeclaration(var) + ";\n";
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
  /*
  std::list<ControlStep> &statelist = scheduler->m_ctrlSteps;
  std::string declare;
  std::string assign;
  std::string ret_state;
  os << " // State sequence " << "\n\n";  
  
  for(auto cs = statelist.begin(); cs!=statelist.end(); cs++)
  {    
    ControlStep& s = *cs;
    std::string this_state = makeHdlStateName(s.getbbName(), s.getId());
    declare += ("logic " + this_state + ";\n");                  
    if (s.isBranch()) {
      if (s.isReturn()) {
        ret_state = this_state;
      }
    } else {
      std::string next_state = makeHdlStateName(s.getbbName(), s.getId()+ 1);
      assign += next_state + VERILOG_ASSIGN + this_state + VERILOG_ENDL;      
    }
  }

  os << declare;
  os << "state_entry0 <= func_start" << VERILOG_ENDL;
  os << VERILOG_CLKPROCESS_TOP(state_process);
  os << assign;
  os << "func_done <= " + ret_state + VERILOG_ENDL;
  os << VERILOG_CLKPROCESS_BOTTOM(state_process);
  */
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

std::string VerilogGenerator::writeOneInstruction(const llvm::Instruction* I)
{
  std::string executor;
  /*
  std::list<ControlStep> &statelist = scheduler->m_ctrlSteps;
  std::map<const llvm::Value*, ValueLifeInfo> &info = scheduler->valueInfoMap;
  char buf[256];
  unsigned size = 0;
  std::string state = makeHdlStateName(s.getbbName(), s.getId());
  
  if (I->isBinaryOp()) {    
    size = sprintf(buf,"binary_unit #(\n");
    executor += std::string(buf, size);     
    size = sprintf(buf,".BITWIDTH (%d),\n", I->getOperand(0)->getType()->getIntegerBitWidth());
    executor += std::string(buf, size); 
    size = sprintf(buf,".OPCODE  (\"%s\"))\n", I->getOpcodeName());
    executor += std::string(buf, size); 
  } else {  
    size = sprintf(buf,"%s_unit ", I->getOpcodeName());
    executor += std::string(buf, size);
    //size = sprintf(buf,"#(.BITWIDTH (%d))n", I->getType()->getIntegerBitWidth());
    //executor += std::string(buf, size); 
  }  
  size = sprintf(buf,"I%lx(\n", reinterpret_cast<uintptr_t>(I));
  executor += std::string(buf, size);
  for(const llvm::Use &use : I->operands())
    { 
      size = sprintf(buf,".operand[%d] (%20s),\n", use.getOperandNo(), (*use).getName().data());
      executor += std::string(buf, size); 
    }
  size = sprintf(buf,".%-10s (%20s),\n","enable", &state[0]);
  executor += std::string(buf, size);
  size = sprintf(buf,".%-10s (%20s),\n","clk", "func_clk");
  executor += std::string(buf, size);   
  size = sprintf(buf,".%-10s (%20s));\n\n","result", I->getName().data());  
  executor += std::string(buf, size);   
  */
  return executor;  
};
