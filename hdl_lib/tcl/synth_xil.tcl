set TOPLEVEL [lindex $argv 0]

#Part number is somewhat arbitrary
set PART xcku3p-ffva676-3-e

create_project -force project proj/project -part $PART
set_property target_language Verilog [current_project]
set_msg_config -id {Synth 8-63} -severity WARNING -new_severity ERROR; # captures assert failure -> fatal

auto_detect_xpm

# Apply timing
read_verilog [glob ../../../hdl_lib/verilog/*]
read_verilog ../${TOPLEVEL}.sv

set xdc [open "constraint.xdc" "w"]
puts $xdc {
create_clock -period 2.500 -name func_clk -waveform {0.000 1.250} [get_ports func_clk]
set_input_delay 0.2 [get_ports * -filter {DIRECTION==IN}]
set_output_delay -clock [get_clocks func_clk] 0.2 [get_ports * -filter {DIRECTION==OUT}]
}
close $xdc

read_xdc constraint.xdc

synth_design -mode out_of_context -top $TOPLEVEL -part $PART -flatten_hierarchy rebuilt -assert


report_timing_summary -delay_type min_max -report_unconstrained -check_timing_verbose -max_paths 10 -input_pins -routable_nets -file report_timing.txt
report_utilization -force -hierarchical -hierarchical_depth 2 -file report_util.txt
write_checkpoint -force ../${TOPLEVEL}.dcp
exit