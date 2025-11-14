// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xlzw.h"

extern XLzw_Config XLzw_ConfigTable[];

XLzw_Config *XLzw_LookupConfig(u16 DeviceId) {
	XLzw_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XLZW_NUM_INSTANCES; Index++) {
		if (XLzw_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XLzw_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XLzw_Initialize(XLzw *InstancePtr, u16 DeviceId) {
	XLzw_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XLzw_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XLzw_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

