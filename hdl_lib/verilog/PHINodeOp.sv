

module PHINodeOp #(
 parameter ParamNumIncomingValues = 1,
 parameter ParamBitWidth = 32
) (
  input  bit clk,
  input  bit enable,
  output bit [ParamBitWidth - 1 : 0] ret,

  input  bit [ParamBitWidth     : 0] input0,
  input  bit [ParamBitWidth     : 0] input1,
  input  bit [ParamBitWidth     : 0] input2,
  input  bit [ParamBitWidth     : 0] input3,
  input  bit [ParamBitWidth     : 0] input4,
  input  bit [ParamBitWidth     : 0] input5,
  input  bit [ParamBitWidth     : 0] input6,
  input  bit [ParamBitWidth     : 0] input7,
  input  bit [ParamBitWidth     : 0] input8,
  input  bit [ParamBitWidth     : 0] input9,
  input  bit [ParamBitWidth     : 0] input10,
  input  bit [ParamBitWidth     : 0] input11,
  input  bit [ParamBitWidth     : 0] input12,
  input  bit [ParamBitWidth     : 0] input13,
  input  bit [ParamBitWidth     : 0] input14,
  input  bit [ParamBitWidth     : 0] input15
);

  bit [ParamBitWidth - 1 : 0] ret_comb;
  bit [ParamBitWidth - 1 : 0] ret_reg;

 DataMux #(ParamNumIncomingValues, ParamBitWidth, ParamBitWidth) 
 largemux ( clk, 
            ret_comb, 
            input0,
            input1,
            input2,
            input3,
            input4,
            input5,
            input6,
            input7,
            input8,
            input9,
            input10,
            input11,
            input12,
            input13,
            input14,
            input15);

  always_ff @(posedge clk) 
  begin 
    if (enable) ret_reg <= ret_comb;    
  end

  assign ret = enable?ret_comb:ret_reg; 

endmodule 
