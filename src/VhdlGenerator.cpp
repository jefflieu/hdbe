
#include "CodeGenerator.hpp"
#include "VhdlTemplate.hpp"

using namespace hdbe;

std::string CodeGenerator::getFunctionName(){
  return g_getStdStringName(scheduler->m_function);
}

std::ostream& VhdlGenerator::writeLibrary(std::ostream& os){
  os << VHDL_DECL_LIBRARY;
  return os;
};


std::ostream& VhdlGenerator::writePorts(std::ostream& os){
  os << VHDL_DECL_ENTITY(getFunctionName());
  os << VHDL_HDBE_PORTS;
  os << VHDL_DECL_ENTITY_END(getFunctionName());    
  return os;
};

std::ostream& VhdlGenerator::writeStateSquence(std::ostream& os){
  std::list<ControlStep> &statelist = scheduler->m_ctrlSteps;
  os << VHDL_PROCESS_TOP(state_process);
  os << VHDL_PROCESS_BOTTOM(state_process);
  return os;
};

void VhdlGenerator::write() 
{
  std::string filename = "Test.vhd";
  std::ofstream os(filename, std::ofstream::out);
  writeLibrary(os);
  writePorts(os);
  os << VHDL_DECL_ARCH(getFunctionName());  
  os << VHDL_ARCH_BEGIN << "\n\n\n";
  writeStateSquence(os);
  os << VHDL_ARCH_END << "\n\n\n";
  os.close();  
};

