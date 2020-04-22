
#include "CodeGenerator.hpp"
#include "VerilogTemplate.hpp"
#include "logging/logger.hpp"

using namespace hdbe;

using String      = std::string;
using Value       = llvm::Value;
using Instruction = llvm::Instruction;

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
  
  std::list<HdlVariable> &variableList = CDI_h->variableList;
  
  os << " // Instructions " << "\n\n";  
  for(auto var_i = variableList.begin(), var_last = variableList.end(); var_i != var_last; ++var_i)
  {    
    //
    if (llvm::Instruction::classof(var_i->getIrValue())) {
      LOG(INFO, *(var_i->getIrValue()));
      os << writeSimpleInstruction(static_cast<Instruction*>(var_i->getIrValue()));
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
  
  if (I->isBinaryOp()) {    
    size = sprintf(buf,"binary_unit #( ");
    instantiate += String(buf, size);     
    size = sprintf(buf,"%d, ", I->getOperand(0)->getType()->getIntegerBitWidth());
    instantiate += String(buf, size); 
    size = sprintf(buf,"\"%s\" )", I->getOpcodeName());
    instantiate += String(buf, size); 
  } else {  
    size = sprintf(buf,"%s_unit ", I->getOpcodeName());
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
      size = sprintf(buf,"%s%s, ", (*use).getName().data(), &tag[0]);
      instantiate += String(buf, size); 
    }

  size = sprintf(buf,"%s%s );\n", I->getName().data(), &tag[0]);  
  instantiate += String(buf, size);   
  
  return instantiate;  
};
