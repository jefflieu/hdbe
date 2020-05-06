

EXAMPLES  :=counter fir shiftreg input_array rgb2yuv
VCD_FILE  :=sim/logs/vlt_dump.vcd
CLEAN_ALL := $(foreach ex, $(EXAMPLES), clean_$(ex))
TEST_ALL  := $(foreach ex, $(EXAMPLES), test_$(ex))

help : 
	@echo "make <example>       : to make everything for the example and run simulation"
	@echo "make clean_<example> : to clean all output files of the example"
	@echo "make all             : to run all examples"


%/$(VCD_FILE) : %/*.c
	cd $* && make -f ../ex.mak


$(TEST_ALL) : test_% :  
	@echo "Running simulation: " $*
	cd $*/sim && ./sim +trace


$(EXAMPLES): % : %/$(VCD_FILE)

$(CLEAN_ALL) : clean_% : 
	cd $* && make clean -f ../ex.mak 


all: $(EXAMPLES)

test_all: $(TEST_ALL)

clean_all: $(CLEAN_ALL)
