

CLANG := clang 
FLAGS := 
CLANG_FLAGS := -O3 -fno-slp-vectorize -S -emit-llvm -c $(FLAGS)
HDBE  := hdbe 
SRCS := $(wildcard *.c)
SIM_BLD := sim

.phony : clean
.phony : ir
.phony : sv

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

all: sim

clean: 
	-rm *.ll -rf
	-rm *.sv -rf
	-rm *.txt -rf
	-rm $(SIM_BLD) -rf
