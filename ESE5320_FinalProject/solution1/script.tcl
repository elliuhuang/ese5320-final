############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project ESE5320_FinalProject
set_top lzw
add_files basic_server/lzw.cpp
open_solution "solution1" -flow_target vitis
set_part {xczu3eg-sbva484-1-i}
create_clock -period 6.667 -name default
#source "./ESE5320_FinalProject/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -format ip_catalog
