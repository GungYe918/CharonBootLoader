#pragma once
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include "BootInfo.h"


BootInfo* InitBootInfo(
    UINT64 FramebufferBase,
    UINT32 Width,
    UINT32 Height,
    UINT32 Pitch,
    UINT64 KernelBase,
    UINT64 KernelSize,
    CHAR8 *CmdlineStr,   // "verbose" 등
    CHAR8 *KernelNameStr // "kernel.elf"
);