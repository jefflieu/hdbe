

EXAMPLES  :=counter fir shiftreg input_array \
             rgb2yuv rgb2yuv_14\
              ctrlflow packetparse \
                loop memport vector_add stack binary_search \
                  sorter
VCD_FILE  :=sim/logs/vlt_dump.vcd
CLEAN_ALL := $(foreach ex, $(EXAMPLES), clean-$(ex))
TEST_ALL  := $(foreach ex, $(EXAMPLES), test-$(ex))
SYN_ALL   := $(foreach ex, $(EXAMPLES), syn-$(ex))

help : 
	@echo "make <example>       : to make everything for the example and run simulation"
	@echo "make clean-<example> : to clean all output files of the example"
	@echo "make all             : to run all examples"


%/$(VCD_FILE) : %/*.c
	cd $* && make -f ../ex.mak


$(TEST_ALL) : test-% : % 
	@echo "Running simulation: " $*
	cd $*/sim && ./sim +trace


$(EXAMPLES): % : %/$(VCD_FILE)

$(CLEAN_ALL) : clean-% : 
	cd $* && make clean -f ../ex.mak 

$(SYN_ALL) : syn-% : 
	cd $* && make $*.dcp -f ../ex.mak

all: $(EXAMPLES)

test-all: $(TEST_ALL)

clean-all: $(CLEAN_ALL)

syn-all: $(SYN_ALL)
