`include "Opcodes.sv"

module loadOp #(
 parameter ParamBitWidth    = 32,
 parameter LoadLatency      = 1
) (
  input  bit clk,
  input  bit enable,
  input  bit [ParamBitWidth - 1 : 0] rdata,
  output bit [ParamBitWidth - 1 : 0] ret
);

  bit [ParamBitWidth - 1 : 0] ret_comb;
  bit [ParamBitWidth - 1 : 0] ret_reg;
  bit [9:1] enable_d;

  assign ret_comb = rdata;

  always_ff @(posedge clk) 
  begin
    enable_d[1] <= enable;
    enable_d[9:2] <= enable_d[8:1];
    if (enable_d[LoadLatency]) ret_reg <= ret_comb;    
  end

  assign ret = enable_d[LoadLatency]?ret_comb:ret_reg; 
  
endmodule 
