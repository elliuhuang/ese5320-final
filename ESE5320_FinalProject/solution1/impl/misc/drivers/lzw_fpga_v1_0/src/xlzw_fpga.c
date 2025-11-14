// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xlzw_fpga.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XLzw_fpga_CfgInitialize(XLzw_fpga *InstancePtr, XLzw_fpga_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XLzw_fpga_Start(XLzw_fpga *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL) & 0x80;
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XLzw_fpga_IsDone(XLzw_fpga *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XLzw_fpga_IsIdle(XLzw_fpga *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XLzw_fpga_IsReady(XLzw_fpga *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XLzw_fpga_Continue(XLzw_fpga *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL) & 0x80;
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL, Data | 0x10);
}

void XLzw_fpga_EnableAutoRestart(XLzw_fpga *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XLzw_fpga_DisableAutoRestart(XLzw_fpga *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_AP_CTRL, 0);
}

void XLzw_fpga_Set_chunk(XLzw_fpga *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_CHUNK_DATA, (u32)(Data));
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_CHUNK_DATA + 4, (u32)(Data >> 32));
}

u64 XLzw_fpga_Get_chunk(XLzw_fpga *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_CHUNK_DATA);
    Data += (u64)XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_CHUNK_DATA + 4) << 32;
    return Data;
}

void XLzw_fpga_Set_chunk_len(XLzw_fpga *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_CHUNK_LEN_DATA, Data);
}

u32 XLzw_fpga_Get_chunk_len(XLzw_fpga *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_CHUNK_LEN_DATA);
    return Data;
}

void XLzw_fpga_Set_compressed(XLzw_fpga *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_DATA, (u32)(Data));
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_DATA + 4, (u32)(Data >> 32));
}

u64 XLzw_fpga_Get_compressed(XLzw_fpga *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_DATA);
    Data += (u64)XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_DATA + 4) << 32;
    return Data;
}

void XLzw_fpga_Set_compressed_length(XLzw_fpga *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_LENGTH_DATA, (u32)(Data));
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_LENGTH_DATA + 4, (u32)(Data >> 32));
}

u64 XLzw_fpga_Get_compressed_length(XLzw_fpga *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_LENGTH_DATA);
    Data += (u64)XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_COMPRESSED_LENGTH_DATA + 4) << 32;
    return Data;
}

void XLzw_fpga_InterruptGlobalEnable(XLzw_fpga *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_GIE, 1);
}

void XLzw_fpga_InterruptGlobalDisable(XLzw_fpga *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_GIE, 0);
}

void XLzw_fpga_InterruptEnable(XLzw_fpga *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_IER);
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_IER, Register | Mask);
}

void XLzw_fpga_InterruptDisable(XLzw_fpga *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_IER);
    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_IER, Register & (~Mask));
}

void XLzw_fpga_InterruptClear(XLzw_fpga *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_fpga_WriteReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_ISR, Mask);
}

u32 XLzw_fpga_InterruptGetEnabled(XLzw_fpga *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_IER);
}

u32 XLzw_fpga_InterruptGetStatus(XLzw_fpga *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XLzw_fpga_ReadReg(InstancePtr->Control_BaseAddress, XLZW_FPGA_CONTROL_ADDR_ISR);
}

