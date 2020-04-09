
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
  os << VHDL_DECL_ARCH(getFunctionName());  
  os << VHDL_ARCH_BEGIN << "\n\n\n";
  os << VHDL_ARCH_END << "\n\n\n";
  return os;
};



void VhdlGenerator::write() 
{
  std::string filename = "Test.vhd";
  std::ofstream os(filename, std::ofstream::out);
  writeLibrary(os);
  writePorts(os);
  os.close();  
};

