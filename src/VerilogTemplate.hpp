
#define VERILOG_ENDL ";\n"
#define VERILOG_ASSIGN " <= "


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
begin: " << #__name << "\n\n"

#define VERILOG_STATEMENT(statement)  "\
  "<< statement << ";\n"


#define VERILOG_CLKPROCESS_BOTTOM(__name) "\n\
end \n"

#define VERILOG_PORTMAP(portstr, varstr)  std::to_string("." + portstr + " ( " + varstr + ")")
#define VERILOG_ARRAY_INDEX(namestr, idx) (namestr + std::to_string('[') + std::to_string(idx) + std::to_string(']'))