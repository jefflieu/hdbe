
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

  bit [ParamBitWidth - 1 : 0] ret_comb;
  bit [ParamBitWidth - 1 : 0] ret_reg;

  generate
  if (ParamOpCode == `OPCODE_ADD) begin 
    assign ret_comb = $signed(lhs) + $signed(rhs);
  end
  else if (ParamOpCode == `OPCODE_SUB) begin 
    assign ret_comb = $signed(lhs) - $signed(rhs);
  end 
  else if (ParamOpCode == `OPCODE_MUL) begin 
    assign ret_comb = $signed(lhs) * $signed(rhs);
  end
  else if (ParamOpCode == `OPCODE_LSHR) begin 
    assign ret_comb = $unsigned(lhs) >> (rhs);
  end
  else if (ParamOpCode == `OPCODE_ASHR) begin 
    assign ret_comb = $signed(lhs) >> (rhs);
  end
  else if (ParamOpCode == `OPCODE_SHL) begin 
    assign ret_comb = $signed(lhs) << (rhs);
  end
  else if (ParamOpCode == `OPCODE_OR) begin 
    assign ret_comb = (lhs) | (rhs);
  end
  else if (ParamOpCode == `OPCODE_AND) begin 
    assign ret_comb = (lhs) & (rhs);
  end
  else if (ParamOpCode == `OPCODE_XOR) begin 
    assign ret_comb = (lhs) ^ (rhs);
  end
  else begin 


  end 
  endgenerate

  // always_ff @(posedge clk) 
  // begin 
  //   if (enable) ret_reg <= ret_comb;    
  // end

  // assign ret = enable?ret_comb:ret_reg; 
  
  assign ret = ret_comb;
endmodule 
