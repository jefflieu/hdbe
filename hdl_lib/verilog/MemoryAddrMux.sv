

module MemoryAddrMux #(
 parameter ParamNumIncomingValues = 1,
 parameter InBitWidth = 32,
 parameter OutBitWidth = InBitWidth
) (
  input  bit clk,
  output bit [InBitWidth - 1 : 0] ret,

  input  bit [InBitWidth     : 0] input0,
  input  bit [InBitWidth     : 0] input1,
  input  bit [InBitWidth     : 0] input2,
  input  bit [InBitWidth     : 0] input3,
  input  bit [InBitWidth     : 0] input4,
  input  bit [InBitWidth     : 0] input5,
  input  bit [InBitWidth     : 0] input6,
  input  bit [InBitWidth     : 0] input7,
  input  bit [InBitWidth     : 0] input8,
  input  bit [InBitWidth     : 0] input9,
  input  bit [InBitWidth     : 0] input10,
  input  bit [InBitWidth     : 0] input11,
  input  bit [InBitWidth     : 0] input12,
  input  bit [InBitWidth     : 0] input13,
  input  bit [InBitWidth     : 0] input14,
  input  bit [InBitWidth     : 0] input15
);
  
 DataMux #(ParamNumIncomingValues, InBitWidth, OutBitWidth) 
 largemux ( clk, 
            ret, 
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
endmodule 

module MemoryWdatMux #(
 parameter ParamNumIncomingValues = 1,
 parameter InBitWidth = 32
) (
  input  bit clk,
  output bit [InBitWidth - 1 : 0] ret,

  input  bit [InBitWidth     : 0] input0,
  input  bit [InBitWidth     : 0] input1,
  input  bit [InBitWidth     : 0] input2,
  input  bit [InBitWidth     : 0] input3,
  input  bit [InBitWidth     : 0] input4,
  input  bit [InBitWidth     : 0] input5,
  input  bit [InBitWidth     : 0] input6,
  input  bit [InBitWidth     : 0] input7,
  input  bit [InBitWidth     : 0] input8,
  input  bit [InBitWidth     : 0] input9,
  input  bit [InBitWidth     : 0] input10,
  input  bit [InBitWidth     : 0] input11,
  input  bit [InBitWidth     : 0] input12,
  input  bit [InBitWidth     : 0] input13,
  input  bit [InBitWidth     : 0] input14,
  input  bit [InBitWidth     : 0] input15
);

DataMux #(ParamNumIncomingValues, InBitWidth, InBitWidth) 
largemux (  clk, 
            ret, 
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
endmodule 

