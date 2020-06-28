// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "Vvector_add.h"

#include "vector_add.h"

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
    Vvector_add* top = new Vvector_add();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;

    const int kCALLS         = 1;
    const int kCLK_PER_CALL  = 1e9;
    const int kMEM_SIZE      = SIZE;
    const int kSTART_TIME    = 10;
    const int kCOMPLETE_TIME = kSTART_TIME + (SIZE+1)*2 + 2;

    int calls = 0, returns = 0, done = 0;
    int Reference[kCALLS];
    int Returns[kCALLS];
    int simErrors = -1;
    s32 a[kMEM_SIZE];
    s32 b[kMEM_SIZE];
    s32 sum_ref[kMEM_SIZE];
    s32 sum[kMEM_SIZE];
    u64 a_addr, b_addr;
    for(int i = 0; i < kMEM_SIZE; i++)
    {
      a[i] = rand();
      b[i] = rand(); 
    }

    vector_add(a, b, sum_ref);

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
              calls++;
            } 
          }
        }
        
        //Sampling 
        if(top->func_clk)
        {
          a_addr       = top->a_raddr;
          b_addr       = top->b_raddr;

          if (top->sum_wren)
            sum[top->sum_waddr] = top->sum_wdata;
          if (top->func_done) 
            returns ++;
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
          top->a_rdata = a[a_addr & (kMEM_SIZE-1)];
          top->b_rdata = b[b_addr & (kMEM_SIZE-1)];
        }


    }
    // Final model cleanup
    top->final();

    //Check
    simErrors = 0;
    for(uint32_t chk = 0; chk < kMEM_SIZE; chk++)
    {
      //VL_PRINTF("%08x Expect %016x, actual %016x (%016x %016x)\n", chk, sum_ref[chk], sum[chk], a[chk], b[chk]);
      if (sum_ref[chk] != sum[chk]) {
        simErrors++;
      }
    }
    simErrors += (main_time > kCOMPLETE_TIME);
    VL_PRINTF("Test : %s with %d errors\n", (simErrors > 0)?"Failed":"Passed", simErrors);
    VL_PRINTF("Time : %06ld - %5s\n", main_time, main_time > kCOMPLETE_TIME?"Failed":"Passed");

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
