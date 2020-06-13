module DataMux #(
 parameter ParamNumIncomingValues = 1,
 parameter InBitWidth = 32,
 parameter OutBitWidth = InBitWidth
) (
  input  bit clk,
  output bit [OutBitWidth - 1 : 0] ret,

  input  bit [InBitWidth : 0] input0,
  input  bit [InBitWidth : 0] input1,
  input  bit [InBitWidth : 0] input2,
  input  bit [InBitWidth : 0] input3,
  input  bit [InBitWidth : 0] input4,
  input  bit [InBitWidth : 0] input5,
  input  bit [InBitWidth : 0] input6,
  input  bit [InBitWidth : 0] input7,
  input  bit [InBitWidth : 0] input8,
  input  bit [InBitWidth : 0] input9,
  input  bit [InBitWidth : 0] input10,
  input  bit [InBitWidth : 0] input11,
  input  bit [InBitWidth : 0] input12,
  input  bit [InBitWidth : 0] input13,
  input  bit [InBitWidth : 0] input14,
  input  bit [InBitWidth : 0] input15
);
  
  bit [InBitWidth-1:0] array [0:15];
  bit [OutBitWidth-1:0] out;
  assign array[0]  =  input0[InBitWidth]? input0[InBitWidth-1:0]:0;
  assign array[1]  =  input1[InBitWidth]? input1[InBitWidth-1:0]:0;
  assign array[2]  =  input2[InBitWidth]? input2[InBitWidth-1:0]:0;
  assign array[3]  =  input3[InBitWidth]? input3[InBitWidth-1:0]:0;
  assign array[4]  =  input4[InBitWidth]? input4[InBitWidth-1:0]:0;
  assign array[5]  =  input5[InBitWidth]? input5[InBitWidth-1:0]:0;
  assign array[6]  =  input6[InBitWidth]? input6[InBitWidth-1:0]:0;
  assign array[7]  =  input7[InBitWidth]? input7[InBitWidth-1:0]:0;
  assign array[8]  =  input8[InBitWidth]? input8[InBitWidth-1:0]:0;
  assign array[9]  =  input9[InBitWidth]? input9[InBitWidth-1:0]:0;
  assign array[10] = input10[InBitWidth]?input10[InBitWidth-1:0]:0;
  assign array[11] = input11[InBitWidth]?input11[InBitWidth-1:0]:0;
  assign array[12] = input12[InBitWidth]?input12[InBitWidth-1:0]:0;
  assign array[13] = input13[InBitWidth]?input13[InBitWidth-1:0]:0;
  assign array[14] = input14[InBitWidth]?input14[InBitWidth-1:0]:0;
  assign array[15] = input15[InBitWidth]?input15[InBitWidth-1:0]:0;
  
  int I;
  /* verilator lint_off COMBDLY */
  always_comb
  begin 
    out = 0;
    for(I=0; I<ParamNumIncomingValues; I=I+1)
      out = out | array[I];
  end 
  assign ret = out[OutBitWidth-1:0];
  /* verilator lint_on COMBDLY */


endmodule 
