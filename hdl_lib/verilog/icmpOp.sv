

module icmpOp #(
 parameter ParamOpCode = "none",
 parameter ParamBitWidth = 32
) (
  input  bit clk,
  input  bit enable,
  input  bit [ParamBitWidth - 1 : 0] lhs,
  input  bit [ParamBitWidth - 1 : 0] rhs,
  output bit ret
);
  
  /*  
    ICMP_EQ    = 32,  ///< equal
    ICMP_NE    = 33,  ///< not equal
    ICMP_UGT   = 34,  ///< unsigned greater than
    ICMP_UGE   = 35,  ///< unsigned greater or equal
    ICMP_ULT   = 36,  ///< unsigned less than
    ICMP_ULE   = 37,  ///< unsigned less or equal
    ICMP_SGT   = 38,  ///< signed greater than
    ICMP_SGE   = 39,  ///< signed greater or equal
    ICMP_SLT   = 40,  ///< signed less than
    ICMP_SLE   = 41,  ///< signed less or equal
  */
  bit ret_comb;
  bit ret_reg;

generate
  if (ParamOpCode == "eq") begin 
    assign ret_comb = (lhs == rhs)?1'b1:1'b0;
  end
  else if (ParamOpCode == "ne") begin 
    assign ret_comb = (lhs != rhs)?1'b1:1'b0;
  end 
  else if (ParamOpCode == "ugt") begin 
    assign ret_comb = (lhs > rhs)?1'b1:1'b0;
  end
  else if (ParamOpCode == "uge") begin 
    assign ret_comb = (lhs >= rhs)?1'b1:1'b0;
  end
  else if (ParamOpCode == "ult") begin 
    assign ret_comb = (lhs < rhs)?1'b1:1'b0;
  end
  else if (ParamOpCode == "ule") begin 
    assign ret_comb = (lhs > rhs)?1'b1:1'b0;
  end
  else if (ParamOpCode == "sgt") begin 
    assign ret_comb = ($signed(lhs) > $signed(rhs))?1'b1:1'b0;
  end
  else if (ParamOpCode == "sge") begin 
    assign ret_comb = ($signed(lhs) >= $signed(rhs))?1'b1:1'b0;
  end
  else if (ParamOpCode == "sle") begin 
    assign ret_comb = ($signed(lhs) < $signed(rhs))?1'b1:1'b0;
  end
  else if (ParamOpCode == "sle") begin 
    assign ret_comb = ($signed(lhs) <= $signed(rhs))?1'b1:1'b0;
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
