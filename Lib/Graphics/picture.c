#include "Picture.h"

void DrawImage(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT8 *rgba, UINTN w, UINTN h, UINTN x, UINTN y) {
    UINT32 *fb = (UINT32 *)(UINTN)gop->Mode->FrameBufferBase;
    UINTN stride = gop->Mode->Info->PixelsPerScanLine;

    for (UINTN dy = 0; dy < h; dy++) {
        for (UINTN dx = 0; dx < w; dx++) {
            UINTN px = x + dx;
            UINTN py = y + dy;
            if (px >= gop->Mode->Info->HorizontalResolution || py >= gop->Mode->Info->VerticalResolution)
                continue;
            UINT32 r = rgba[4 * (dy * w + dx) + 0];
            UINT32 g = rgba[4 * (dy * w + dx) + 1];
            UINT32 b = rgba[4 * (dy * w + dx) + 2];
            fb[py * stride + px] = (0xFF << 24) | (b << 16) | (g << 8) | r; // BGRA
        }
    }
}