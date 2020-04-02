#pragma once

#include <string>

#include "types.hpp"

class BaseClass {
  private: 
    const std::string m_idString;

  public : 
    BaseClass() : m_idString("DefaultID") {};
    ~BaseClass() {};

    std::string getId() {return m_idString;}

};
