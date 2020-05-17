#pragma once 

#include <string>

#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/ADT/StringExtras.h"


namespace hdbe {

using Value = llvm::Value;
using String = std::string;

String makeHdlName(String name);
String makeHdlStateName(int id);
String getValueHdlName(Value * val);

}
