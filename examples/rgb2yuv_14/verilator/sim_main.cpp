// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "Vrgb2yuv_14.h"

// Current simulation time (64-bit unsigned)
vluint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp() {
    return main_time;  // Note does conversion to real, to match SystemC
}

#include "rgb2yuv_14.h"


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
    Vrgb2yuv_14* top = new Vrgb2yuv_14();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;
    top->rgb[0] = 123;
    top->rgb[1] = 456;
    top->rgb[2] = 789;
    s14 coef[9];
    
    coef[0] = (0.257*(1<<FRAC));
    coef[1] = (0.504*(1<<FRAC));
    coef[2] = (0.098*(1<<FRAC));
    coef[3] = (-0.148*(1<<FRAC));
    coef[4] = (-0.291*(1<<FRAC));
    coef[5] = (0.439*(1<<FRAC));
    coef[6] = (0.439*(1<<FRAC));
    coef[7] = (-0.368*(1<<FRAC));
    coef[8] = (-0.071*(1<<FRAC));

    top->coef[0] = coef[0];
    top->coef[1] = coef[1];
    top->coef[2] = coef[2];
    top->coef[3] = coef[3];
    top->coef[4] = coef[4];
    top->coef[5] = coef[5];
    top->coef[6] = coef[6];
    top->coef[7] = coef[7];
    top->coef[8] = coef[8];
  
    const int kCALLS         = 10;
    const int kSTART_TIME    = 10;
    const int kCLK_PER_CALL  = 1;
    int calls = 0, returns = 0;
    s14 Reference[kCALLS*3];
    u14 ReferenceRgb[3];
    s14 Returns[kCALLS*3];

    int simErrors = -1;
   
    //VL_PRINTF("Expected value %d\n", ref);
    while(true) {
        main_time++;  // Time passes...

        // Toggle a fast (time/2 period) clock
        top->func_clk = main_time & 0x1;

        // Toggle control signals on an edge that doesn't correspond
        // to where the controls are sampled
        if (!top->func_clk) {
          if (main_time >= kSTART_TIME) {
            top->func_start = ( calls<kCALLS && ( ((main_time - kSTART_TIME) >> 1) % kCLK_PER_CALL == 0) );  // Assert function call
            ReferenceRgb[0] = rand();
            ReferenceRgb[1] = rand();
            ReferenceRgb[2] = rand();
            top->rgb[0]   = ReferenceRgb[0];
            top->rgb[1]   = ReferenceRgb[1];
            top->rgb[2]   = ReferenceRgb[2];
            if (top->func_start)
            {
              rgb2yuv_14(ReferenceRgb, coef, &Reference[calls*3]);
              calls += 1;
            } 
          }
        }

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each.)
        top->eval();
        if (top->func_done && top->func_clk) {
          Returns[returns*3 + 0] = top->yuv[0];
          Returns[returns*3 + 1] = top->yuv[1];
          Returns[returns*3 + 2] = top->yuv[2];
          returns ++;
          if (returns == kCALLS) break;
        }

        // Read outputs
        //VL_PRINTF("[%" VL_PRI64 "d] clk=%x func_start=%x func_done = %x func_ret = %d \n",
        //          main_time, top->func_clk, top->func_start, top->func_done, top->func_ret);
    
    };

    // Final model cleanup
    top->final();
    //Check
    simErrors = 0;
    for(uint32_t chk = 0; chk < kCALLS*3; chk++)
    {
      if (Reference[chk] != Returns[chk]) simErrors++;
    }
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
