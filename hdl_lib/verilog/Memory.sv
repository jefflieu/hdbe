

module Memory #(
 parameter AddrWidth = 8,
 parameter DataWidth = 64
) (
  input  bit clk,
  input  bit wren,
  input  bit [AddrWidth-1:0] waddr,
  input  bit [DataWidth-1:0] wdata,
  input  bit rden,
  input  bit [AddrWidth-1:0] raddr,
  output  bit [DataWidth-1:0] rdata
);
  
  bit [DataWidth-1:0] mem [0:2**AddrWidth-1];
  always_ff @(posedge clk) 
  begin : proc
    if (wren) 
      mem[waddr] <= wdata;
    if (rden)
      rdata <= mem[raddr];
  end
 
endmodule 

