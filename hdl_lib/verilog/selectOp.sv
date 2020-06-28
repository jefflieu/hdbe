

module selectOp #(
 parameter ParamOpCode = "none",
 parameter ParamBitWidth = 32
) (
  input  bit clk,
  input  bit enable,
  input  bit cond,
  input  bit [ParamBitWidth - 1 : 0] trueval,
  input  bit [ParamBitWidth - 1 : 0] falseval,
  output bit [ParamBitWidth - 1 : 0] ret
);

  bit [ParamBitWidth - 1 : 0] ret_comb;
  bit [ParamBitWidth - 1 : 0] ret_reg; 

  assign ret_comb = cond?trueval:falseval;

  // always_ff @(posedge clk) 
  // begin 
  //   if (enable) ret_reg <= ret_comb;    
  // end

  // assign ret = enable?ret_comb:ret_reg; 

  assign ret = ret_comb;

endmodule 
