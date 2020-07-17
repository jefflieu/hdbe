This HDBE - Hardware Description Backend project implements a HDL (currently SystemVerilog/Verilog) generator using LLVM IR. 
It relies on LLVM as front end for analysis and transformations
It parses the IR and tries to understand what's the objective of the code and generate the SystemVerilog/Verilog accordingly

Installation and required dependency

- Checkout latest llvm from https://github.com/llvm/llvm-project
- Build and install llvm-project, this builds clang and other libraries which are statically linked into HDBE
- Checkout and install Verilator from : https://www.veripool.org/wiki/verilator. Verilator is used in simulation of the generated Verilog. The HDBE includes several examples for quick regression test and evaluation of how HDBE works. The examples are built using gcc/clang/verilator
- Build and add hdbe executable into your path
- Go to example, go to any of the example: 
+ make clean -f ../ex.mak
+ make sim -f ../ex.mak
The script would compile c file into .ll file then generate .sv file. It then uses verilator to simulate the model with the testbench written in example/<example_name>/verilator.
This is a work in progress and only a subset of C constructs are supported.
It's quite fun to DIY HSL. 

