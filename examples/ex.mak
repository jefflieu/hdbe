

CLANG := clang 
FLAGS := -O3 -fno-slp-vectorize -S -emit-llvm -c
HDBE  := hdbe 
SRCS := $(wildcard *.c)
SIM_BLD := sim

.phony : clean
.phony : ir
.phony : sv

%.sv : %.ll
	$(HDBE) $^ $*

%.ll : %.c
	$(CLANG) $(FLAGS) -o $@ $^

all: $(SRCS:.c=.sv) sim/Makefile
	make -C $(SIM_BLD)

ir : $(SRCS:.c=.ll)

sv : $(SRCS:.c=.sv)

sim/Makefile:
	-mkdir $(SIM_BLD)
	cd $(SIM_BLD) && cmake ../verilator 


clean: 
	-rm *.ll -rf
	-rm *.sv -rf
	-rm $(SIM_BLD) -rf
