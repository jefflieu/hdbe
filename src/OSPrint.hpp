#pragma once

#include <iostream>
#include "ControlStep.hpp"

namespace hdbe {

std::ostream& operator<<(std::ostream& os, const ControlStep& cs);
std::string to_hexstring(uintptr_t ptr, char h = 'H');
}