

CLANG := clang 
FLAGS := -DHDBE
CLANG_FLAGS := -O3 -fno-slp-vectorize -S -emit-llvm -c $(FLAGS)
HDBE  := hdbe 
SRCS := $(wildcard *.c)
SIM_BLD := sim
SYN_BLD := syn
SYN_DCP := ../../../hdl_lib/tcl/synth_xil.tcl

.phony : clean
.phony : ir
.phony : sv
.phony : dcp

%.sv : %.ll
	$(HDBE) $^ $*

%.ll : %.c
	$(CLANG) $(CLANG_FLAGS) -o $@ $^

$(SIM_BLD)/sim: $(SRCS:.c=.sv) sim/Makefile
	make -C $(SIM_BLD)

ir : $(SRCS:.c=.ll)

sv : $(SRCS:.c=.sv)

sim/Makefile:
	-mkdir $(SIM_BLD)
	cd $(SIM_BLD) && cmake ../verilator 

sim: $(SIM_BLD)/sim
	cd $(SIM_BLD) && ./sim +trace

%.dcp: %.sv
	-mkdir $(SYN_BLD)
	cd $(SYN_BLD) && vivado -mode tcl -source $(SYN_DCP) -tclargs $*

dcp : $(SRCS:.c=.dcp)

all: sim

clean: 
	-rm *.ll -rf
	-rm *.sv -rf
	-rm *.txt -rf
	-rm *.dcp -rf
	-rm $(SIM_BLD) -rf
	-rm $(SYN_BLD) -rf
