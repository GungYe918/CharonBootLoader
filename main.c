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
#include "Lib/Elf/loadElf.h"
#include "Lib/Graphics/Screen/InitScreen.h"

EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;
EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;

#define KERNEL_PATH L"\\kernel\\kernel.elf"


EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) { 
    InitScreen(SystemTable, 800, 600, GetPixelColor(0, 0, 255));

    /* TODO: fix error... maybe...
    UINT8 *image;
    UINT32 w, h;
    if (LoadPngFromDisk(ImageHandle, L"\\logo.png", &image, &w, &h)) {
        DrawImage(gop, image, w, h, 100, 100);
    } else {
        gST->ConOut->OutputString(gST->ConOut, L"PNG load failed\r\n");
    }*/

    Print(L"Hello! Welcome to the Charon World!!!\n");
    Print(L"\n1\t2\t3\t4\t5\t6\n\n");

    
    LoadElfFile(ImageHandle);


    while (1);
    return EFI_SUCCESS;
}


//__attribute__((section(".reloc")))
//const unsigned char dummy_reloc[1] = { 0 };