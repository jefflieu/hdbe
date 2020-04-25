// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "VTest.h"

// Current simulation time (64-bit unsigned)
vluint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp() {
    return main_time;  // Note does conversion to real, to match SystemC
}

extern "C" int MultiplyAccumulate8(int, int, int, int, int, int, int, int);

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
    VTest* top = new VTest();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;
    top->a = 1;
    top->b = 2;
    top->c = 3;
    top->d = 4;
    top->e = 5;
    top->f = 6;
    top->g = 7;
    top->h = 8;

    int ref = MultiplyAccumulate8(top->a, top->b, top->c, top->d, top->e, top->f, top->g, top->h);
    VL_PRINTF("Expected value %d\n", ref);

    // Simulate until $finish
    //while (!Verilated::gotFinish()) {
    while (!top->func_done) {
        main_time++;  // Time passes...

        // Toggle a fast (time/2 period) clock
        top->func_clk = !top->func_clk;

        // Toggle control signals on an edge that doesn't correspond
        // to where the controls are sampled; in this example we do
        // this only on a negedge of clk, because we know
        // reset is not sampled there.
        if (!top->func_clk) {
            if (main_time == 4) {
                top->func_start = 1;  // Assert reset
            } else {
                top->func_start = 0;  // Deassert reset
            }
        }

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each.)
        top->eval();

        // Read outputs
        VL_PRINTF("[%" VL_PRI64 "d] clk=%x rstl=%x func_done = %x func_ret = %d \n",
                  main_time, top->func_clk, top->func_start, top->func_done, top->func_ret);
    }
    if (top->func_ret == ref) VL_PRINTF("PASSED\n"); else VL_PRINTF("FAILED\n");
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
