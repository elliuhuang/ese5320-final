// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xlzw_fpga.h"

extern XLzw_fpga_Config XLzw_fpga_ConfigTable[];

XLzw_fpga_Config *XLzw_fpga_LookupConfig(u16 DeviceId) {
	XLzw_fpga_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XLZW_FPGA_NUM_INSTANCES; Index++) {
		if (XLzw_fpga_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XLzw_fpga_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XLzw_fpga_Initialize(XLzw_fpga *InstancePtr, u16 DeviceId) {
	XLzw_fpga_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XLzw_fpga_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XLzw_fpga_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

