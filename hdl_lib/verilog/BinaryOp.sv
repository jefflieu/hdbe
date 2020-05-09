
`include "Opcodes.sv"

module BinaryOp #(
 parameter ParamOpCode = "__none__",
 parameter ParamBitWidth = 32
) (
  input  bit clk,
  input  bit enable,
  input  bit [ParamBitWidth - 1 : 0] lhs,
  input  bit [ParamBitWidth - 1 : 0] rhs,
  output bit [ParamBitWidth - 1 : 0] ret
);


  generate
  if (ParamOpCode == `OPCODE_ADD) begin 
    assign ret = $signed(lhs) + $signed(rhs);
  end
  else if (ParamOpCode == `OPCODE_SUB) begin 
    assign ret = $signed(lhs) - $signed(rhs);
  end 
  else if (ParamOpCode == `OPCODE_MUL) begin 
    assign ret = $signed(lhs) * $signed(rhs);
  end
  else if (ParamOpCode == `OPCODE_LSHR) begin 
    assign ret = $unsigned(lhs) >> (rhs);
  end
  else if (ParamOpCode == `OPCODE_ASHR) begin 
    assign ret = $signed(lhs) >> (rhs);
  end
  else if (ParamOpCode == `OPCODE_SHL) begin 
    assign ret = $signed(lhs) << (rhs);
  end
  else if (ParamOpCode == `OPCODE_OR) begin 
    assign ret = (lhs) | (rhs);
  end
  else if (ParamOpCode == `OPCODE_AND) begin 
    assign ret = (lhs) & (rhs);
  end
  else if (ParamOpCode == `OPCODE_XOR) begin 
    assign ret = (lhs) ^ (rhs);
  end
  else begin 


  end 
  endgenerate
 
endmodule 
