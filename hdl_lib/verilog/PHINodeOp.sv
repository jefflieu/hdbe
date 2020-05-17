

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
  
  bit [ParamBitWidth-1:0] array [0:15];
  assign array[0]  =  input0[ParamBitWidth]? input0[ParamBitWidth-1:0]:0;
  assign array[1]  =  input1[ParamBitWidth]? input1[ParamBitWidth-1:0]:0;
  assign array[2]  =  input2[ParamBitWidth]? input2[ParamBitWidth-1:0]:0;
  assign array[3]  =  input3[ParamBitWidth]? input3[ParamBitWidth-1:0]:0;
  assign array[4]  =  input4[ParamBitWidth]? input4[ParamBitWidth-1:0]:0;
  assign array[5]  =  input5[ParamBitWidth]? input5[ParamBitWidth-1:0]:0;
  assign array[6]  =  input6[ParamBitWidth]? input6[ParamBitWidth-1:0]:0;
  assign array[7]  =  input7[ParamBitWidth]? input7[ParamBitWidth-1:0]:0;
  assign array[8]  =  input8[ParamBitWidth]? input8[ParamBitWidth-1:0]:0;
  assign array[9]  =  input9[ParamBitWidth]? input9[ParamBitWidth-1:0]:0;
  assign array[10] = input10[ParamBitWidth]?input10[ParamBitWidth-1:0]:0;
  assign array[11] = input11[ParamBitWidth]?input11[ParamBitWidth-1:0]:0;
  assign array[12] = input12[ParamBitWidth]?input12[ParamBitWidth-1:0]:0;
  assign array[13] = input13[ParamBitWidth]?input13[ParamBitWidth-1:0]:0;
  assign array[14] = input14[ParamBitWidth]?input14[ParamBitWidth-1:0]:0;
  assign array[15] = input15[ParamBitWidth]?input15[ParamBitWidth-1:0]:0;
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
  int I;
  /* verilator lint_off COMBDLY */
  always_comb
  begin 
    ret = 0;
    for(I=0; I<ParamNumIncomingValues; I=I+1)
      ret = ret | array[I];
  end 
  /* verilator lint_on COMBDLY */


endmodule 
