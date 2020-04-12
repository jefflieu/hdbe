
#include "CodeGenerator.hpp"
#include "VerilogTemplate.hpp"

using namespace hdbe;

std::string VerilogGenerator::writeHdlObjDeclaration(HdlObject& obj)
{   
    std::string decl;
    decl = (obj.m_property.stype == HdlSignalType::portType)?"input ":"  ";
    switch (obj.m_property.vtype)
    {
      case HdlVectorType::scalarType : decl += "logic [" + std::to_string(obj.m_property.bitwidth-1) + ":0] " + makeHdlName(obj.m_name) + ",\n";
                                       break;
      case HdlVectorType::memoryType : break;
      case HdlVectorType::arrayType  : decl += "logic [" + std::to_string(obj.m_property.bitwidth-1) + ":0] " + makeHdlName(obj.m_name) + "[" + std::to_string(obj.m_property.arraylength-1) + ":0],\n";
      default : break;
    }
  return decl;
}


std::ostream& VerilogGenerator::writePorts(std::ostream& os){
  os << VERILOG_DECL_MODULE(getFunctionName());
    
  //Iterate over analyzer.m_portList;
  for(auto I = analyzer->m_portList.begin(), E = analyzer->m_portList.end(); I!=E; ++I)
  {
    HdlPort& port = *I;
    os << writeHdlObjDeclaration(port);
  }

  os << VERILOG_HDBE_PORTS;
  return os;
};

std::ostream& VerilogGenerator::writeStateSquence(std::ostream& os){
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
  os << VERILOG_CLKPROCESS_BOTTOM(state_process);
  os << "func_done <= " + ret_state + VERILOG_ENDL;
  return os;
};

void VerilogGenerator::write() 
{
  std::string filename = "Test.v";
  std::ofstream os(filename, std::ofstream::out);  
  writePorts(os);
  
  writeStateSquence(os);
  

  os << VERILOG_DECL_MODULE_END(getFunctionName());    
  os.close();  
};

