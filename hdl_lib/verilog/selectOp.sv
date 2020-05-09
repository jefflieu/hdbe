

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


  assign ret = cond?trueval:falseval;
endmodule 
