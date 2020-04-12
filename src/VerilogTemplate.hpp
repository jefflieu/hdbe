
#define VERILOG_ENDL ";\n"
#define VERILOG_ASSIGN " <= "


#define VERILOG_DECL_MODULE(__name) \
"module " << __name << " ( \n\n" 

#define VERILOG_HDBE_PORTS "\
  input  logic func_start , \n\
  output logic func_done  , \n\
  input  logic func_clk   , \n\
  input  logic func_rst   \n\n\
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
