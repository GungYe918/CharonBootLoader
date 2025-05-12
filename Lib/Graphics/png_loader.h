#pragma once
#include <Uefi.h>
#include <stdbool.h>

bool LoadPngFromDisk(
    EFI_HANDLE ImageHandle,
    CHAR16 *filename,
    UINT8 **rgba_out,
    UINT32 *width,
    UINT32 *height
);