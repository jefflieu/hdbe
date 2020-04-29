#pragma once 

#include <string>

#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/ADT/StringExtras.h"


namespace hdbe {

using Value = llvm::Value;
using String = std::string;

std::string makeHdlName(std::string name);
std::string makeHdlStateName(std::string bbName, int id);
std::string getValueHdlName(Value * val);

}
