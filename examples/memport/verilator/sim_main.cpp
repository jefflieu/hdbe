// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "Vmemport.h"

#include "memport.h"

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
    Vmemport* top = new Vmemport();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;

    const int kCALLS         = 200;
    const int kCLK_PER_CALL  = 1e9;
    const int kMEM_SIZE      = 128;
    const int kSTART_TIME    = 10;
    int calls = 0, returns = 0, done = 0;
    int Reference[kCALLS];
    int Returns[kCALLS];
    int simErrors = -1;
    u16 data[kMEM_SIZE];
    u16 mem_raddr;

    for(int i = 0; i < kMEM_SIZE; i++)
    {
      data[i] = rand();
    }
    //VL_PRINTF("Expected value %d\n", ref);

    while (true) {
        main_time++;  // Time passes...

        // Toggle a fast (time/2 period) clock
        top->func_clk = main_time & 0x1;

        // Toggle control signals on an edge that doesn't correspond
        // to where the controls are sampled
        if (!top->func_clk) { 
          if (main_time >= kSTART_TIME) {
            top->func_start = (calls < kCALLS && ((main_time == kSTART_TIME) || done));
            if (top->func_start)
            {
              top->data = data[rand() & (kMEM_SIZE - 1)];//Search value
              top->size = kMEM_SIZE;
              Reference[calls] = memport(data, top->data, top->size);
              calls++;
            } 
          }
        }

        //Sampling
        if (top->func_clk)
        {
          mem_raddr  = top->mem_raddr; 
          
          if (top->func_done) {
            Returns[returns] = top->func_ret;
            returns++;
          }
          done = top->func_done;
          if (returns == kCALLS) break;
        }
       

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each.)
        top->eval();

        //Drive
        if(top->func_clk)
        {
          top->mem_rdata = data[mem_raddr % kMEM_SIZE];
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
      if (Reference[chk] != Returns[chk]) { 
        simErrors++;
        VL_PRINTF("Error at %d, %x vs %x\n", chk, Reference[chk], Returns[chk]);
      }
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
