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
#define KERNEL_PATH L"\\kernel\\kernel.elf"

__attribute__((used))
EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) { 
    InitScreen(SystemTable, 800, 600, GetPixelColor(0, 0, 255));
    Print(L"Hello! Welcome to the Charon World!!!\n");
#ifdef MDE_CPU_AARCH64
    // AArch64에서는 더미 코드만 실행
    Print(L"[Info] AARCH64 architecture detected.\n");
    Print(L"[Info] Skipping actual kernel loading and jumping.\n");
    Print(L"UEFI services remain active. Exiting early for test.\n");

    while(1);
    return EFI_SUCCESS;
#endif
    
    EFI_STATUS Status;
    UINT64 EntryPoint = 0;
    UINT64 KernelBase = 0;
    UINT64 KernelSize = 0;

    // 커널 로드
    Status = LoadKernel(ImageHandle, KERNEL_PATH, &EntryPoint, &KernelBase, &KernelSize);
    if (EFI_ERROR(Status)) {
        Print(L"[FATAL] Kernel loading failed: %r\n", Status);
        Print(L"Press any key to exit...\n");
        gST->ConIn->Reset(gST->ConIn, FALSE);
        EFI_INPUT_KEY Key;
        while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY);
        return Status;
    }
    
    Print(L"\n=== KERNEL LOADED SUCCESSFULLY ===\n");
    Print(L"Entry Point: 0x%lx\n", EntryPoint);
    Print(L"Kernel Base: 0x%lx\n", KernelBase);
    Print(L"Kernel Size: 0x%lx bytes (%d KB)\n", KernelSize, (UINT32)(KernelSize / 1024));
    
    // 커널 진입점 검증
    UINT8 *EntryBytes = (UINT8*)(UINTN)EntryPoint;
    Print(L"Entry point instruction bytes: ");
    for (int i = 0; i < 16; i++) {
        Print(L"%02x ", EntryBytes[i]);
    }
    Print(L"\n");

    // 메모리 정보 수집
    UINT32 TotalMemMB = ScanMemMap_MB();
    Print(L"Total Memory: %d MB\n", TotalMemMB);

    // 프레임버퍼 정보
    UINT64 FrameBufferBase = (UINT64)(UINTN)gGop->Mode->FrameBufferBase;
    UINT32 Width = gGop->Mode->Info->HorizontalResolution;
    UINT32 Height = gGop->Mode->Info->VerticalResolution;
    UINT32 Pitch = gGop->Mode->Info->PixelsPerScanLine * 4;

    Print(L"Framebuffer: 0x%lx (%dx%d, pitch=%d)\n", FrameBufferBase, Width, Height, Pitch);

    // FreeBSD 호환 BootInfo 구조체 생성
    BootInfo* BootData = InitBootInfo(
        FrameBufferBase,
        Width, Height, Pitch, 
        ConvertPixelFormat(gGop->Mode->Info->PixelFormat),
        KernelBase, KernelSize, 
        "verbose",      // 커맨드라인 옵션
        "kernel.elf"    // 커널 이름
    );
    
    if (!BootData) {
        Print(L"[FATAL] Failed to create BootInfo structure\n");
        return EFI_ABORTED;
    }

    Print(L"\n=== BOOTINFO PREPARED ===\n");
    Print(L"BootInfo at: 0x%lx\n", (UINT64)(UINTN)BootData);
    Print(L"Version: 0x%x\n", BootData->bi_version);
    Print(L"Memory size: %d MB\n", BootData->bi_memsize);
    
    // 모듈 정보 확인
    PreloadedFile *module = (PreloadedFile *)(UINTN)(BootData->bi_modulep);
    if (module) {
        Print(L"Kernel module: %u at 0x%lx (size: 0x%lx)\n", 
              module->f_name ? module->f_name : "unknown",
              module->f_addr, module->f_size);
    }

    Print(L"\n=== READY TO EXIT BOOT SERVICES ===\n");
    Print(L"This will transfer control from UEFI to the kernel.\n");
    Print(L"All UEFI services will be unavailable after this point.\n");
    
    // 3초 대기
    for (int i = 3; i > 0; i--) {
        Print(L"Exiting in %d seconds...\n", i);
        gBS->Stall(1000000); // 1초 대기
    }

    // ExitBootServices 실행
    Print(L"Calling ExitBootServices...\n");

    UINT8 *KernelCode = (UINT8*)(UINTN)EntryPoint;
    Print(L"[Debug] Kernel code at entry point: %02x %02x %02x %02x %02x %02x %02x %02x\n",
          KernelCode[0], KernelCode[1], KernelCode[2], KernelCode[3],
          KernelCode[4], KernelCode[5], KernelCode[6], KernelCode[7]);

    
    Print(L"BootData pointer: 0x%lx\n", (UINT64)(UINTN)BootData);
    Print(L"BootData->bi_framebuffer_addr = 0x%lx\n", BootData->bi_framebuffer_addr);
    Print(L"Password Hash:\n");
    for (int i = 0; i < 32; ++i) {
        Print(L"%02x ", BootData->bi_pwd_hash[i]);
        if ((i + 1) % 16 == 0)
            Print(L"\n");
    }


    Status = ExitBootServicesWithRetry(ImageHandle, NULL);
    if (EFI_ERROR(Status)) {
        Print(L"[FATAL] ExitBootServices failed: %r\n", Status);
        return Status;
    }

    // 이 시점에서 Print는 사용할 수 없음 (UEFI 서비스 종료됨)
    
    // 커널로 제어권 이전
    JumpToKernel(EntryPoint, BootData);

    // 여기에 도달하면 안됨
    while (1) {
        __asm__ __volatile__("hlt");
    }
    
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
