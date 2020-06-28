`include "Opcodes.sv"

module CastOp #(
 parameter ParamOpCode = "none",
 parameter ParamBitWidth    = 32,
 parameter ReturnBitWidth   = 32 //Doesn't have to be set
) (
  input  bit clk,
  input  bit enable,
  input  bit [ParamBitWidth - 1 : 0] lhs,
  output bit [ReturnBitWidth - 1 : 0] ret
);

  bit [ReturnBitWidth - 1 : 0] ret_comb;
  bit [ReturnBitWidth - 1 : 0] ret_reg;

  generate
  if (ParamOpCode == `OPCODE_ZEXT) begin 
    assign ret_comb = ReturnBitWidth'($unsigned(lhs));
  end
  else if (ParamOpCode == `OPCODE_SEXT) begin 
    assign ret_comb = ReturnBitWidth'($signed(lhs));
  end 
  else if (ParamOpCode == `OPCODE_TRUNC) begin 
    assign ret_comb = ReturnBitWidth'($signed(lhs));
  end
  else begin 


  end 
  endgenerate


  // always_ff @(posedge clk) 
  // begin
  //   if (enable) ret_reg <= ret_comb;    
  // end

  //assign ret = enable?ret_comb:ret_reg; 
  assign ret = ret_comb;
 
endmodule 
