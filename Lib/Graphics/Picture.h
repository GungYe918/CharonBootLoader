#pragma once
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>


void DrawImage(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT8 *rgba, UINTN w, UINTN h, UINTN x, UINTN y);