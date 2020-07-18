/*
  Copyright 2020 
  Jeff Lieu <lieumychuong@gmail.com>
*/

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

#define VERILOG_CLKPROCESS_TOP(__name) String("\
always@(posedge func_clk) \n\
begin //"  __name "\n\n")

#define VERILOG_ALWAYS_COMB(__name) String("\
always_comb \n\
begin //"  __name "\n\n")

#define VERILOG_ALWAYS_CLK_BLK(NAME, CLK) ("always_ff@(posedge " + CLK + ") \nbegin //" + NAME + "\n\n")
#define VERILOG_ALWAYS_COMB_BLK(NAME) ("always_comb() \nbegin //" + NAME + "\n\n")

#define VERILOG_RANGE(a, b) ("[" + std::to_string(a) + ":" + std::to_string(b) + "]")

#define VERILOG_VAR_DECL(_type, name) (std::string(#_type" ") + name)

#define VERILOG_VEC_DECL(_type, size, name) (std::string(#_type" ") + VERILOG_RANGE(size-1,0) + name)

#define VERILOG_ARR_DECL(_type, dim1, name, dim2) (std::string(#_type" ") + VERILOG_RANGE(dim1-1, 0) + name + VERILOG_RANGE(dim2-1, 0))

#define VERILOG_STATEMENT(statement)  "\
  "<< statement << ";\n"


#define VERILOG_CLKPROCESS_BOTTOM(__name) String("\n\
end //" __name "\n")

#define VERILOG_PORTMAP(portstr, varstr)  std::to_string("." + portstr + " ( " + varstr + ")")
#define VERILOG_ARRAY_INDEX(namestr, idx) (namestr + std::string("[") + std::to_string(idx) + std::string("]"))

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
#define VERILOG_COMMENT           "//"
#define VERILOG_SIGNAL_CONCAT(a, b) ("{" + a + "," + b + "}")

#define VERILOG_CODE_SECTION(a) String("/*\n") + String(a) + String("\n*/\n")

#define CYCLE_TAG(n)  ("_" + std::to_string(n))
#define VALID_TAG      "_valid"
#define LOOP_TAG       "_loop"
#define STR_TAG(str)   #str
#define NEW_LINE       "\n"
#define INDENT(n)      String(2*n, ' ')

#define MEMOBJ_WREN(obj)  (obj.name + "_wren")
#define MEMOBJ_RDEN(obj)  (obj.name + "_rden")
#define MEMOBJ_WDATA(obj) (obj.name + "_wdata")
#define MEMOBJ_RDATA(obj) (obj.name + "_rdata")
#define MEMOBJ_RADDR(obj) (obj.name + "_raddr")
#define MEMOBJ_WADDR(obj) (obj.name + "_waddr")

#define FUNC_CLK_NET   "func_clk"
#define FUNC_START_NET "func_start"
#define FUNC_DONE_NET  "func_done"
#define FUNC_RET_NET   "func_ret"


