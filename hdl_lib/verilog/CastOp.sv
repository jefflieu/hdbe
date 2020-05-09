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


  generate
  if (ParamOpCode == `OPCODE_ZEXT) begin 
    assign ret = ReturnBitWidth'($unsigned(lhs));
  end
  else if (ParamOpCode == `OPCODE_SEXT) begin 
    assign ret = ReturnBitWidth'($signed(lhs));
  end 
  else if (ParamOpCode == `OPCODE_TRUNC) begin 
    assign ret = ReturnBitWidth'($signed(lhs));
  end
  else begin 


  end 
  endgenerate
 
endmodule 
