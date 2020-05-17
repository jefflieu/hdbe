
#include "HDLUtil.hpp"

using namespace hdbe;
String hdbe::makeHdlName(String name) {
      String invalid_char = ".?+-*";
      String new_string;
      static int Id = 0;
      if (name.length() == 0) {
        new_string = "s" + std::to_string(Id);
        Id++;
      } else  
        for(auto i = name.begin(); i != name.end(); i++)
          {          
            if(invalid_char.find(*i) != String::npos)                     
              new_string.push_back('_');
            else 
              new_string.push_back(*i);  
          }
      return new_string;
}


String hdbe::makeHdlStateName(int id) {
  return makeHdlName(String("state_") + std::to_string(id));
}

String hdbe::getValueHdlName(Value *val)
{
  if (val->hasName())
    return val->getName().str();
  else if (llvm::ConstantInt::classof(val))
    return String("k" + llvm::utohexstr(reinterpret_cast<uintptr_t>(val)) + "_" + 
            llvm::utostr(static_cast<llvm::ConstantInt*>(val)->getSExtValue()) + "d");
  else
    return String("s" + llvm::utohexstr(reinterpret_cast<uintptr_t>(val)));  
}