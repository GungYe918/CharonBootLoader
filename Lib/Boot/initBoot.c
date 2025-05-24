#include "BootInfo.h"
#include "../Utils/mem.h"
#include "../Utils/Print.h"

extern EFI_BOOT_SERVICES *gBS;

// BootInfo구조체 초기화 함수수
BootInfo* InitBootInfo(
    UINT64 FramebufferBase,
    UINT32 Width,
    UINT32 Height,
    UINT32 Pitch,
    UINT64 KernelBase,
    UINT64 KernelSize,
    CHAR8 *CmdlineStr,   // 예: "verbose"
    CHAR8 *KernelNameStr // 예: "kernel.elf"
) {
    EFI_STATUS Status;

    BootInfo* BootData = NULL;
    PreloadedFile* KModule = NULL;
    CHAR8 *CmdLineBuf;
    CHAR8 *KNameBuf;

    // BootInfo 구조체 할당 & 초기화
    Status = gBS->AllocatePool(EfiLoaderData, sizeof(BootInfo), (VOID**)&BootData);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to Allocate Memory [BootInfo] : %r\n", Status);
        return NULL;
    }
    ZeroMem(BootData, sizeof(BootInfo));


    // PreLoadedFile 구조체 초기화
    Status = gBS->AllocatePool(EfiLoaderData, sizeof(PreloadedFile), (VOID**)&KModule);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to Allocate Memory [PreLoadedFile] : %r\n", Status);
        return NULL;
    }
    ZeroMem(KModule, sizeof(PreloadedFile));


    // 커맨드 라인 구조체 초기화
    UINTN CmdLen = 0;
    while (CmdlineStr[CmdLen] != '\0') 
        CmdLen++;
    Status = gBS->AllocatePool(EfiLoaderData, CmdLen + 1, (VOID**)&CmdLineBuf);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to Allocate Memory [CmdLine] : %r\n", Status);
    }
    for (UINTN i = 0; i <= CmdLen; ++i) {
        CmdLineBuf[i] = CmdlineStr[i];
    }


    // 커널 이름 복사
    UINTN NameLen = 0;
    while (KernelNameStr[NameLen] != '\0') NameLen++;
    Status = gBS->AllocatePool(EfiLoaderData, NameLen + 1, (VOID**)&KNameBuf);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to Allocate Memory [Kernel Name] : %r\n", Status);
        return NULL;
    }
    for (UINTN i = 0; i <= NameLen; ++i) KNameBuf[i] = KernelNameStr[i];


    // PreLoadedFile 구조체 초기화
    KModule->f_name = KNameBuf;
    KModule->f_type = "elf kernel";
    KModule->f_addr = KernelBase;
    KModule->f_size = KernelSize;
    KModule->f_next = NULL;

    // BootInfo 초기화
    BootData->bi_version                = BOOTINFO_VERSION;
    BootData->bi_kernelname             = (UINT32)(UINTN)KNameBuf;
    BootData->bi_framebuffer_addr       = FramebufferBase;
    BootData->bi_framebuffer_width      = Width;
    BootData->bi_framebuffer_height     = Height;
    BootData->bi_framebuffer_pitch      = Pitch;
    BootData->bi_howto                  = RB_VERBOSE;
    BootData->bi_memsize                = ScanMemMap_MB();
    BootData->bi_modulep                = (UINT64)(UINTN)KModule;
    BootData->bi_envp                   = 0;
    BootData->bi_cmdline                = (UINT64)(UINTN)CmdLineBuf;
    BootData->bi_kernelbase             = KernelBase;

    

    return BootData;



}   