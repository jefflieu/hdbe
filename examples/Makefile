

EXAMPLES:=counter fir shiftreg input_array
VCD_FILE:=sim/logs/vlt_dump.vcd

%/$(VCD_FILE) : %/*.c
	cd $* && make -f ../ex.mak
	@echo "Running simulation"
	cd $*/sim && ./sim +trace


$(EXAMPLES): % : %/$(VCD_FILE)

$(foreach ex, $(EXAMPLES), clean_$(ex)) : clean_% : 
	cd $* && make clean -f ../ex.mak 


all: $(EXAMPLES)

clean_all: $(foreach ex, $(EXAMPLES), clean_$(ex))
