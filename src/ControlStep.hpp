#pragma once 
#include <string> 
#include <list> 
#include <iostream> 

#include "BaseClass.hpp"
#include "OSPrint.hpp"

namespace hdbe {

class ControlStep : public BaseClass {
  
  private: 
    bool isBranchNode = false;
    std::list<Const_Instruction_h> m_instrList;    
    std::string m_Name;
    int m_id = 0;
  
  public: 
    ControlStep (const std::string& name, int id) :  m_Name(name), m_id(id) {}
    ~ControlStep() {}

    std::string getName() {return this->m_Name;}
    int  getId() {return this->m_id;}
    bool isBranch() {return isBranchNode;};
    void setBranch(bool branch) {isBranchNode = branch;};
    void setId(int _id) {m_id = _id;}
    bool isEntry() {return (m_id == 0);}
    
    bool addInstruction(Const_Instruction_h instruction) {      
      m_instrList.push_back(instruction);
    }

    std::ostream& print(std::ostream& os) const {      
      os << "Step: " << m_Name << " Branch: " << isBranchNode <<'\n';
      for(auto I = m_instrList.begin(); I != m_instrList.end(); ++I){   
        std::string name = g_getStdStringName(*I);
        std::string hash_name = 'S' + std::to_string((unsigned long long)*I & 0xffffffff);
        os << (*I) << " " << (name.length()>0?name:hash_name) << " " << (*I)->getOpcodeName() << '\n'; 
      }
      return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const ControlStep& cs);    
};

}
