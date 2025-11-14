// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XLZW_H
#define XLZW_H

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
#include "xlzw_hw.h"

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
} XLzw_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XLzw;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XLzw_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XLzw_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XLzw_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XLzw_ReadReg(BaseAddress, RegOffset) \
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
int XLzw_Initialize(XLzw *InstancePtr, u16 DeviceId);
XLzw_Config* XLzw_LookupConfig(u16 DeviceId);
int XLzw_CfgInitialize(XLzw *InstancePtr, XLzw_Config *ConfigPtr);
#else
int XLzw_Initialize(XLzw *InstancePtr, const char* InstanceName);
int XLzw_Release(XLzw *InstancePtr);
#endif

void XLzw_Start(XLzw *InstancePtr);
u32 XLzw_IsDone(XLzw *InstancePtr);
u32 XLzw_IsIdle(XLzw *InstancePtr);
u32 XLzw_IsReady(XLzw *InstancePtr);
void XLzw_Continue(XLzw *InstancePtr);
void XLzw_EnableAutoRestart(XLzw *InstancePtr);
void XLzw_DisableAutoRestart(XLzw *InstancePtr);

void XLzw_Set_chunk(XLzw *InstancePtr, u64 Data);
u64 XLzw_Get_chunk(XLzw *InstancePtr);
void XLzw_Set_chunk_len(XLzw *InstancePtr, u32 Data);
u32 XLzw_Get_chunk_len(XLzw *InstancePtr);
void XLzw_Set_compressed(XLzw *InstancePtr, u64 Data);
u64 XLzw_Get_compressed(XLzw *InstancePtr);
void XLzw_Set_compressed_length(XLzw *InstancePtr, u64 Data);
u64 XLzw_Get_compressed_length(XLzw *InstancePtr);

void XLzw_InterruptGlobalEnable(XLzw *InstancePtr);
void XLzw_InterruptGlobalDisable(XLzw *InstancePtr);
void XLzw_InterruptEnable(XLzw *InstancePtr, u32 Mask);
void XLzw_InterruptDisable(XLzw *InstancePtr, u32 Mask);
void XLzw_InterruptClear(XLzw *InstancePtr, u32 Mask);
u32 XLzw_InterruptGetEnabled(XLzw *InstancePtr);
u32 XLzw_InterruptGetStatus(XLzw *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
