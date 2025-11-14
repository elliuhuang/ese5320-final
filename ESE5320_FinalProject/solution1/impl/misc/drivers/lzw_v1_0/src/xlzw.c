// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xlzw.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XLzw_CfgInitialize(XLzw *InstancePtr, XLzw_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XLzw_Start(XLzw *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL) & 0x80;
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XLzw_IsDone(XLzw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XLzw_IsIdle(XLzw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XLzw_IsReady(XLzw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XLzw_Continue(XLzw *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL) & 0x80;
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL, Data | 0x10);
}

void XLzw_EnableAutoRestart(XLzw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XLzw_DisableAutoRestart(XLzw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_AP_CTRL, 0);
}

void XLzw_Set_chunk(XLzw *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_CHUNK_DATA, (u32)(Data));
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_CHUNK_DATA + 4, (u32)(Data >> 32));
}

u64 XLzw_Get_chunk(XLzw *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_CHUNK_DATA);
    Data += (u64)XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_CHUNK_DATA + 4) << 32;
    return Data;
}

void XLzw_Set_chunk_len(XLzw *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_CHUNK_LEN_DATA, Data);
}

u32 XLzw_Get_chunk_len(XLzw *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_CHUNK_LEN_DATA);
    return Data;
}

void XLzw_Set_compressed(XLzw *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_DATA, (u32)(Data));
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_DATA + 4, (u32)(Data >> 32));
}

u64 XLzw_Get_compressed(XLzw *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_DATA);
    Data += (u64)XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_DATA + 4) << 32;
    return Data;
}

void XLzw_Set_compressed_length(XLzw *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_LENGTH_DATA, (u32)(Data));
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_LENGTH_DATA + 4, (u32)(Data >> 32));
}

u64 XLzw_Get_compressed_length(XLzw *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_LENGTH_DATA);
    Data += (u64)XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_COMPRESSED_LENGTH_DATA + 4) << 32;
    return Data;
}

void XLzw_InterruptGlobalEnable(XLzw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_GIE, 1);
}

void XLzw_InterruptGlobalDisable(XLzw *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_GIE, 0);
}

void XLzw_InterruptEnable(XLzw *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_IER);
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_IER, Register | Mask);
}

void XLzw_InterruptDisable(XLzw *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_IER);
    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_IER, Register & (~Mask));
}

void XLzw_InterruptClear(XLzw *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XLzw_WriteReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_ISR, Mask);
}

u32 XLzw_InterruptGetEnabled(XLzw *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_IER);
}

u32 XLzw_InterruptGetStatus(XLzw *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XLzw_ReadReg(InstancePtr->Control_BaseAddress, XLZW_CONTROL_ADDR_ISR);
}

