
#include "HDLUtil.hpp"

using namespace hdbe;
std::string hdbe::makeHdlName(std::string name) {
      std::string invalid_char = ".?+-*";
      std::string new_string;
      static int Id = 0;
      if (name.length() == 0) {
        new_string = "s" + std::to_string(Id);
        Id++;
      } else  
        for(auto i = name.begin(); i != name.end(); i++)
          {          
            if(invalid_char.find(*i) != std::string::npos)                     
              new_string.push_back('_');
            else 
              new_string.push_back(*i);  
          }
      return new_string;
}


std::string hdbe::makeHdlStateName(std::string bbName, int id) {
  return makeHdlName("state_" + bbName + std::to_string(id));
}