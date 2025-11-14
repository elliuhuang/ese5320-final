// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XLZW_FPGA_H
#define XLZW_FPGA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xlzw_fpga_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u32 Control_BaseAddress;
} XLzw_fpga_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XLzw_fpga;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XLzw_fpga_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XLzw_fpga_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XLzw_fpga_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XLzw_fpga_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XLzw_fpga_Initialize(XLzw_fpga *InstancePtr, u16 DeviceId);
XLzw_fpga_Config* XLzw_fpga_LookupConfig(u16 DeviceId);
int XLzw_fpga_CfgInitialize(XLzw_fpga *InstancePtr, XLzw_fpga_Config *ConfigPtr);
#else
int XLzw_fpga_Initialize(XLzw_fpga *InstancePtr, const char* InstanceName);
int XLzw_fpga_Release(XLzw_fpga *InstancePtr);
#endif

void XLzw_fpga_Start(XLzw_fpga *InstancePtr);
u32 XLzw_fpga_IsDone(XLzw_fpga *InstancePtr);
u32 XLzw_fpga_IsIdle(XLzw_fpga *InstancePtr);
u32 XLzw_fpga_IsReady(XLzw_fpga *InstancePtr);
void XLzw_fpga_Continue(XLzw_fpga *InstancePtr);
void XLzw_fpga_EnableAutoRestart(XLzw_fpga *InstancePtr);
void XLzw_fpga_DisableAutoRestart(XLzw_fpga *InstancePtr);

void XLzw_fpga_Set_chunk(XLzw_fpga *InstancePtr, u64 Data);
u64 XLzw_fpga_Get_chunk(XLzw_fpga *InstancePtr);
void XLzw_fpga_Set_chunk_len(XLzw_fpga *InstancePtr, u32 Data);
u32 XLzw_fpga_Get_chunk_len(XLzw_fpga *InstancePtr);
void XLzw_fpga_Set_compressed(XLzw_fpga *InstancePtr, u64 Data);
u64 XLzw_fpga_Get_compressed(XLzw_fpga *InstancePtr);
void XLzw_fpga_Set_compressed_length(XLzw_fpga *InstancePtr, u64 Data);
u64 XLzw_fpga_Get_compressed_length(XLzw_fpga *InstancePtr);

void XLzw_fpga_InterruptGlobalEnable(XLzw_fpga *InstancePtr);
void XLzw_fpga_InterruptGlobalDisable(XLzw_fpga *InstancePtr);
void XLzw_fpga_InterruptEnable(XLzw_fpga *InstancePtr, u32 Mask);
void XLzw_fpga_InterruptDisable(XLzw_fpga *InstancePtr, u32 Mask);
void XLzw_fpga_InterruptClear(XLzw_fpga *InstancePtr, u32 Mask);
u32 XLzw_fpga_InterruptGetEnabled(XLzw_fpga *InstancePtr);
u32 XLzw_fpga_InterruptGetStatus(XLzw_fpga *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
