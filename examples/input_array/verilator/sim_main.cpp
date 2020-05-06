// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "Vinput_array.h"

#include "input_array.h"

// Current simulation time (64-bit unsigned)
vluint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp() {
    return main_time;  // Note does conversion to real, to match SystemC
}



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
    Vinput_array* top = new Vinput_array();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;
    top->din[0] = 1;
    top->din[1] = 2;
    top->din[2] = 3;
    top->din[3] = 4;


    const int kCALLS         = 10;
    const int kCLK_PER_CALL  = 1;
    int calls = 0, returns = 0;
    int Reference[kCALLS];
    int Returns[kCALLS];
    int simErrors = -1;
   
    //VL_PRINTF("Expected value %d\n", ref);

    while (! (top->func_done and returns == kCALLS) ) {
        main_time++;  // Time passes...

        // Toggle a fast (time/2 period) clock
        top->func_clk = main_time & 0x1;

        // Toggle control signals on an edge that doesn't correspond
        // to where the controls are sampled
        if (!top->func_clk && main_time >= 4) {
            top->func_start = ( (calls<kCALLS) && ( (main_time >> 1) % kCLK_PER_CALL == 0) ) ? 1 : 0;  // Assert function call
            top->din[0]   = rand();  
            top->din[1]   = rand();  
            top->din[2]   = rand();  
            top->din[3]   = rand();  
            
            if (top->func_start)
            {
              Reference[calls] = input_array(top->din);
              calls++;
            } 
        }

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each.)
        top->eval();

        if (top->func_done && top->func_clk) 
        {
          Returns[returns] = top->func_ret;
          returns++;
        }

        // Read outputs
        //VL_PRINTF("[%" VL_PRI64 "d] clk=%x din=%d func_start=%x func_done = %x func_ret = %d \n",
        //          main_time, top->func_clk, top->din[0], top->func_start, top->func_done, top->func_ret);
    }
    // Final model cleanup
    top->final();

    //Check
    simErrors = 0;
    for(uint32_t chk = 0; chk < kCALLS; chk++)
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
