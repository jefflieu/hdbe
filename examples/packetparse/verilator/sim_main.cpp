// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>
#include <cstdlib>
// Include model header, generated from Verilating "top.v"
#include "Vpacketparse.h"

// Current simulation time (64-bit unsigned)
vluint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp() {
    return main_time;  // Note does conversion to real, to match SystemC
}

#define CHECK(COND, MSG) do {  if (! (COND)) {VL_PRINTF("ERROR! %s\n", MSG); exit(1);}} while(0)

#include "packetparse.h"

int main(int argc, char** argv, char** env) {
    // This is a more complicated example, please also see the simpler examples/make_hello_c.

    // Prevent unused variable warnings
    if (0 && argc && argv && env) {}

    // Set debug level, 0 is off, 9 is highest presently used
    // May be overridden by commandArgs
    Verilated::debug(0);

    // Randomization reset policy
    // May be overridden by commandArgs
    Verilated::randReset(2);

    // Verilator must compute traced signals
    Verilated::traceEverOn(true);

    // Pass arguments so Verilated code can see them, e.g. $value$plusargs
    // This needs to be called before you create any model
    Verilated::commandArgs(argc, argv);

    // Create logs/ directory in case we have traces to put under it
    Verilated::mkdir("logs");

    // Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
    Vpacketparse* top = new Vpacketparse();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;
    u64 packet_data[20] = { WORD(0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x11, 0x22),
                            WORD(0x33, 0x44, 0x55, 0x66, 0x08, 0x00, 0x00, 0x00), 
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00),
                            WORD(00, 00, 00, 00, 00, 00, 00, 00)};
    u64 ref_dst_mac, ref_src_mac;
    u16 ref_eth_type, ref_eth_length;

    const int kCALLS         = 20;
    const int kSTART_TIME    = 10;
    const int kCLK_PER_CALL  = 1;
    const int kPKT_LENGTH    = 20;
    int calls = 0, returns = 0;
    int Reference[kCALLS];
    int Returns[kCALLS];
    int simErrors = -1;
    int word_cnt = 0;
    //VL_PRINTF("Expected value %d\n", ref);

    while (! (top->func_done and word_cnt >= kPKT_LENGTH) ) {
        main_time++;  // Time passes...

        // Toggle a fast (time/2 period) clock
        top->func_clk = main_time & 0x1;

        // Toggle control signals on an edge that doesn't correspond
        // to where the controls are sampled
        if (!top->func_clk) {
          if (main_time >= kSTART_TIME) {
            top->func_start = ( calls<kCALLS && ( ((main_time - kSTART_TIME) >> 1) % kCLK_PER_CALL == 0) );  // Assert function call 
            top->pktdata    = packet_data[word_cnt];
            top->sop        = word_cnt == 0?1:0;
            top->eop        = word_cnt == kPKT_LENGTH-1?1:0;
            if (top->func_start)
            {
              packetparse(top->pktdata, top->sop, top->eop, &ref_dst_mac, &ref_src_mac, &ref_eth_type, &ref_eth_length);
              calls++;
              word_cnt++;
            } 
          }
        }

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each.)
        top->eval();
        if (top->func_clk) {
          if (top->mac_dst_valid) CHECK(top->mac_dst[0] == ref_dst_mac, "MAC ADDRESS MISMATCH");
          if (top->mac_src_valid) CHECK(top->mac_src[0] == ref_src_mac, "MAC ADDRESS MISMATCH");
          if (top->eth_length_valid) CHECK(top->eth_length[0] == ref_eth_length, "ETH TYPE MISMATCH");
          if (top->eth_type_valid  ) CHECK(top->eth_type[0] == ref_eth_type, "ETH LENGTH MISMATCH");
        }

        // Read outputs
        //VL_PRINTF("[%" VL_PRI64 "d] clk=%x func_start=%x func_done = %x func_ret = %d \n",
        //          main_time, top->func_clk, top->func_start, top->func_done, top->func_ret);
    }
    // Final model cleanup
    top->final();
    
    VL_PRINTF("Test : %s with %d errors\n", (simErrors > 0)?"Failed":"Passed", simErrors);


    //  Coverage analysis (since test passed)
#if VM_COVERAGE
    Verilated::mkdir("logs");
    VerilatedCov::write("logs/coverage.dat");
#endif

    // Destroy model
    delete top; top = NULL;

    // Fin
    exit(simErrors);
}
