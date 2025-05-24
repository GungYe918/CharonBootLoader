#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <Protocol/LoadedImage.h>
//#include "Lib/Graphics/png_loader.h" <-- implement soon,,, maybe,,,
#include "Lib/Elf/Elf.h"
#include "Lib/Utils/Print.h"
#include "Lib/Graphics/Image/Picture.h"
#include "Lib/Utils/mem.h"
#include "Lib/Load/Load.h"
#include "Lib/Graphics/Screen/InitScreen.h"
#include "Lib/Boot/initBoot.h"

EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;
EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;
#define KERNEL_PATH L"\\kernel.elf"


EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) { 
    InitScreen(SystemTable, 1024, 768, GetPixelColor(0, 0, 255));
    Print(L"Hello! Welcome to the Charon World!!!\n");
    
    //LoadElfFile(ImageHandle, KERNEL_PATH);
    EFI_STATUS Status;
    UINT64 EntryPoint;
    UINT64 KernelBase = 0;
    UINT64 KernelSize = 0;

    Status = LoadKernel(ImageHandle, KERNEL_PATH, &EntryPoint, &KernelBase, &KernelSize);
    if (EFI_ERROR(Status)) {
        Print(L"Something Happening...");
    }
    Print(L"[ FINAL CHECK!! ] ENTRY:::::<0x%lx>\n", EntryPoint);
    Print(L"KernelBase = 0x%lx\n", KernelBase);
    Print(L"KernelSize = 0x%lx\n", KernelSize);

    UINT32 memmap = ScanMemMap_MB();
    Print(L"MemMapSize = 0x%lx\n", memmap);

    // 프레임 버퍼 초기화
    UINT64 FrameBufferBase      = (UINT64)(UINTN)gGop->Mode->FrameBufferBase;
    UINT32 Width                = gGop->Mode->Info->HorizontalResolution;
    UINT32 Height               = gGop->Mode->Info->VerticalResolution;
    UINT32 Pitch                = gGop->Mode->Info->PixelsPerScanLine * 4;

    // BootInfo 초기화
    BootInfo* BootData = InitBootInfo(
        FrameBufferBase,
        Width, Height,
        Pitch, 
        KernelBase,
        KernelSize, 
        "verbose",
        "kernel.elf"
    );
    if (!BootData) {
        Print(L"[Error] Failed to create BootData. Panic!!!\n");
        return EFI_ABORTED;
    } else {
        Print(L"Success!!!\n");
    }


    while (1);
    return EFI_SUCCESS;
}

 /* TODO: fix error... maybe...
    UINT8 *image;
    UINT32 w, h;
    if (LoadPngFromDisk(ImageHandle, L"\\logo.png", &image, &w, &h)) {
        DrawImage(gop, image, w, h, 100, 100);
    } else {
        gST->ConOut->OutputString(gST->ConOut, L"PNG load failed\r\n");
    }*/