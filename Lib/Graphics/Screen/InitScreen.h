#pragma once
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include "../../Boot/BootInfo.h"


extern EFI_SYSTEM_TABLE *gST;
extern EFI_BOOT_SERVICES *gBS;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;

EFI_STATUS InitScreen(EFI_SYSTEM_TABLE *SystemTable, UINT32 width, UINT32 height, UINT32 color);
PixelFormat ConvertPixelFormat(EFI_GRAPHICS_PIXEL_FORMAT fmt);
UINT32 GetPixelColor(UINT32 r, UINT32 g, UINT32 b);
void FillScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 color);


