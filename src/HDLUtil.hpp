#pragma once 

#include <string>

namespace hdbe {

std::string makeHdlName(std::string name);
std::string makeHdlStateName(std::string bbName, int id);

}
