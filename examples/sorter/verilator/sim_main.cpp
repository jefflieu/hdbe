// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "Vsorter.h"

#include "sorter.h"

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
    Vsorter* top = new Vsorter();  // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

    // Set some inputs
    
    top->func_clk = 0;
    top->func_start = 0;

    const int kCALLS         = 1;
    const int kCLK_PER_CALL  = 1e9;
    const int kMEM_SIZE      = 64;
    const int kSTART_TIME    = 10;
    const int kTIME_OUT      = kMEM_SIZE*kMEM_SIZE*10*2;
    const int kCOMPLETE_TIME = 33300;
    int calls = 0, returns = 0, done = 0;
    int Reference[kCALLS];
    int Returns[kCALLS];
    int simErrors = -1;
    s32 data_reference[kMEM_SIZE];
    s32 data[kMEM_SIZE];
    s32 data_rdata;
    u16 data_raddr;
    u16 data_waddr;
    s32 data_fixed[kMEM_SIZE] = {1, 2, 4, 9, 3, 4, 6, 10, 12, 15, 20, 4, 5, 9, 8, 7};

    for(int i = 0; i < kMEM_SIZE; i++)
    {
      data_reference[i] = rand() % 100;
      //data_reference[i] = data_fixed[i];
      data[i] = data_reference[i];
    }
    //VL_PRINTF("Expected value %d\n", ref);

    while (main_time < kTIME_OUT) {
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
              top->size = kMEM_SIZE;
              sorter(data_reference, (u13) top->size);
              calls++;
              VL_PRINTF("Function called \n");
            } 
          }
        }

        //Sampling
        if (top->func_clk)
        {
          data_raddr  = top->data_raddr; 
          data_rdata  = data[data_raddr % kMEM_SIZE];
          data_waddr  = top->data_waddr;
          if (top->func_done) {
            Returns[returns] = top->func_ret;
            returns++;
          }
          if (top->data_wren) data[data_waddr % kMEM_SIZE] = top->data_wdata; 
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
          //top->data_rdata = data[data_raddr % kMEM_SIZE];
          top->data_rdata = data_rdata;
        }

        // Read outputs
        //VL_PRINTF("[%" VL_PRI64 "d] clk=%x din=%d func_start=%x func_done = %x func_ret = %d \n",
        //          main_time, top->func_clk, top->din[0], top->func_start, top->func_done, top->func_ret);
    }
    // Final model cleanup
    top->final();

    //Check
    simErrors = 0;
    for(uint32_t chk = 0; chk < kMEM_SIZE; chk++)
    {
      if (data_reference[chk] != data[chk]) { 
        simErrors++;
        VL_PRINTF("Error at %d, %08x vs %08x\n", chk, data_reference[chk], data[chk]);
      }
    }
    simErrors += (main_time > kCOMPLETE_TIME);
    VL_PRINTF("Test : %s with %d errors\n", (simErrors > 0)?"Failed":"Passed", simErrors);
    VL_PRINTF("Time : %06ld - %5s\n", main_time, main_time > kCOMPLETE_TIME?"Failed":"Passed");
    for(uint32_t chk = 0; chk < kMEM_SIZE; chk++)
    {
      VL_PRINTF("%c%d", ((chk&0xf)==0)?'\n':' ',data[chk]);
    }
    VL_PRINTF("\n");
    
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
