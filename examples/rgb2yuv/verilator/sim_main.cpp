// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "Vrgb2yuv.h"

// Current simulation time (64-bit unsigned)
vluint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp() {
    return main_time;  // Note does conversion to real, to match SystemC
}


typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

extern "C" void rgb2yuv(u16* rgb, s16 * coef, s16* yuv);

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
    Vrgb2yuv* top = new Vrgb2yuv();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;
    top->rgb[0] = 123;
    top->rgb[1] = 456;
    top->rgb[2] = 789;
    top->coef[0] = 1024;
    top->coef[1] = 0;
    top->coef[2] = 0;
    top->coef[3] = 0;
    top->coef[4] = 1024;
    top->coef[5] = 0;
    top->coef[6] = 0;
    top->coef[7] = 0;
    top->coef[8] = 1024;

    const int kSAMPLES = 2;
    int ref[kSAMPLES];
    int ret[kSAMPLES];
    int sample;
   
    //VL_PRINTF("Expected value %d\n", ref);

    // Simulate until $finish
    sample = 0;
    //while (!Verilated::gotFinish()) {
    while (! (top->func_done and sample == kSAMPLES) ) {
        main_time++;  // Time passes...

        // Toggle a fast (time/2 period) clock
        top->func_clk = main_time & 0x1;

        // Toggle control signals on an edge that doesn't correspond
        // to where the controls are sampled; in this example we do
        // this only on a negedge of clk, because we know
        // reset is not sampled there.
        if (!top->func_clk) {
          if (main_time == 4) {
            top->func_start = 1;  // Assert reset
          }
	      }

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each.)
        top->eval();

        if (top->func_done && top->func_clk) {
          ret[sample] = top->func_ret;
          sample++;
        }

        // Read outputs
        VL_PRINTF("[%" VL_PRI64 "d] clk=%x func_start=%x func_done = %x func_ret = %d \n",
                  main_time, top->func_clk, top->func_start, top->func_done, top->func_ret);
    }
    // Final model cleanup
    top->final();

    //  Coverage analysis (since test passed)
#if VM_COVERAGE
    Verilated::mkdir("logs");
    VerilatedCov::write("logs/coverage.dat");
#endif

    // Destroy model
    delete top; top = NULL;

    // Fin
    exit(0);
}
