
#define VERILOG_ENDL ";\n"



#define VERILOG_DECL_MODULE(__name) \
"module " << __name << " ( \n\n" 

#define VERILOG_HDBE_PORTS "\
input  bit func_start , \n\
output bit func_done  , \n\
input  bit func_clk   , \n\
input  bit func_rst   \n\n\
); \n"

#define VERILOG_DECL_MODULE_END(__name) "\
\n\
endmodule //" << __name << "\n\n"

#define VERILOG_CLKPROCESS_TOP(__name) "\
always@(posedge func_clk) \n\
begin //" << __name << "\n\n"

#define VERILOG_RANGE(a, b) ("[" + std::to_string(a) + ":" + std::to_string(b) + "]")

#define VERILOG_VAR_DECL(_type, name) (std::string(#_type" ") + name)

#define VERILOG_VEC_DECL(_type, size, name) (std::string(#_type" ") + VERILOG_RANGE(size-1,0) + name)

#define VERILOG_ARR_DECL(_type, dim1, name, dim2) (std::string(#_type" ") + VERILOG_RANGE(dim1-1, 0) + name + VERILOG_RANGE(dim2-1, 0))

#define VERILOG_STATEMENT(statement)  "\
  "<< statement << ";\n"


#define VERILOG_CLKPROCESS_BOTTOM(__name) "\n\
end //" << __name << "\n"

#define VERILOG_PORTMAP(portstr, varstr)  std::to_string("." + portstr + " ( " + varstr + ")")
#define VERILOG_ARRAY_INDEX(namestr, idx) (namestr + std::to_string('[') + std::to_string(idx) + std::to_string(']'))

#define VERILOG_ASSIGN_STATEMENT  "assign "
#define VERILOG_LOGICAL_AND       " && "
#define VERILOG_LOGICAL_OR        " || "
#define VERILOG_LOGICAL_NOT       " !  "
#define VERILOG_COMPARE_EQUAL     " == "
#define VERILOG_CONT_ASSIGN       " = "
#define VERILOG_ASSIGN            " <= "
#define VERILOG_OUTPUT            "output "
#define VERILOG_INPUT             "input  "
#define VERILOG_IF(cond)             "if ( " + cond + " )"
#define VERILOG_ELSE              " else "
#define VERILOG_BEGIN             "begin\n"
#define VERILOG_END               "end\n"

#define VERILOG_CODE_SECTION(a) "\n\n/*\n" a "\n*/\n"
