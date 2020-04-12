
#define VHDL_DECL_LIBRARY "\n\
library ieee; \n\
use ieee.std_logic_1164.all; \n\
use ieee.std_logic_unsigned.all;\n\
use ieee.numeric_std.all; \n\n"


#define VHDL_DECL_ENTITY(__name) \
"entity " << __name << " is \n\n\
  port ( \n" 


#define VHDL_HDBE_PORTS "\
    func_start                :  in bit;\n\
    func_done                 : out bit;\n\
    func_clk                  :  in std_logic;\n\
    func_rst                  :  in bit \n"

#define VHDL_DECL_ENTITY_END(__name) \
"  );\n\n\
end " << __name << ";"

#define VHDL_DECL_ARCH(__entityname) \
"\n\n\
architecture auto_gen of " << __entityname << " is \n\n"

#define VHDL_ARCH_BEGIN "\nbegin\n\n"

#define VHDL_ARCH_END   "end auto_gen;\n\n"


#define VHDL_PROCESS_TOP(__name) "\
\
  " << #__name << ": process(func_clk) \n\
  begin \n\
    if rising_edge(func_clk) then \n"

#define VHDL_PROCESS_BOTTOM(__name) "\
    end if; \n\
  end process; --" << #__name << "\n\n"


#define VHDL_STATEMENT_L1(statement) "\
      " << statement << ";\n"

#define VHDL_STATEMENT_L2(statement) "\
        " << statement << ";\n"

#define VHDL_STATEMENT_L3(statement) "\
          " << statement << ";\n"

#define VHDL_IF_STATEMENT(__cond, __then, __else) "\
if " << __cond << " then " << "\n"\
<< __then << "\n\
else \n"\
<< __else << "\n\
end if;\n"
