############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project lzw_fpga
add_files basic_server/lzw_fpga.cpp
open_solution "solution1" -flow_target vitis
set_part {xczu3eg-sbva484-1-i}
create_clock -period 10 -name default
#source "./lzw_fpga/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -format ip_catalog
