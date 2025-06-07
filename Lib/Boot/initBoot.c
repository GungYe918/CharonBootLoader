#include "BootInfo.h"
#include "../Utils/mem.h"
#include "../Utils/Print.h"
#include "../Security/PwdHash.h"


extern EFI_BOOT_SERVICES *gBS;

// BootInfo구조체 초기화 함수
BootInfo* InitBootInfo(
    UINT64 FramebufferBase,
    UINT32 Width,
    UINT32 Height,
    UINT32 Pitch,
    PixelFormat pixelFormat,
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
        return NULL;
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
    BootData->bi_pixel_format           = pixelFormat;
    BootData->bi_howto                  = RB_VERBOSE;
    BootData->bi_memsize                = ScanMemMap_MB();
    BootData->bi_modulep                = (UINT64)(UINTN)KModule;
    BootData->bi_envp                   = 0;
    BootData->bi_cmdline                = (UINT64)(UINTN)CmdLineBuf;
    BootData->bi_kernelbase             = KernelBase;
    RegisterHash(BootData, kPasswordHash, 32);

    

    return BootData;



} 

EFI_STATUS ExitBootServicesWithRetry(EFI_HANDLE ImageHandle, UINTN *MapKeyOut) {
    EFI_STATUS Status;
    EFI_MEMORY_DESCRIPTOR *MemMap = NULL;
    UINTN MemMapSize = 0;
    UINTN LocalMapKey; // Use a local variable for MapKey
    UINTN DescSize;
    UINT32 DescVersion;
    BOOLEAN ExitBootServicesSucceeded = FALSE;

    // 1단계: 사이즈 요청
    Status = gBS->GetMemoryMap(&MemMapSize, NULL, &LocalMapKey, &DescSize, &DescVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        Print(L"[Error] GetMemoryMap failed: %r\n", Status);
        return Status;
    }

RetryAllocateAndGetMap:
    // 2단계: 버퍼 할당 (여유분 포함)
    // 이전 MemMap이 있다면 해제 (ExitBootServices 실패 시에만 도달)
    if (MemMap != NULL) {
        gBS->FreePool(MemMap);
        MemMap = NULL;
    }
    MemMapSize += 2 * DescSize; // 약간의 추가 공간 확보
    Status = gBS->AllocatePool(EfiLoaderData, MemMapSize, (VOID **)&MemMap);
    if (EFI_ERROR(Status)) {
        Print(L"[Error] AllocatePool for MemoryMap failed: %r\n", Status);
        return Status;
    }

    // 3단계: 실제 맵 가져오기
    Status = gBS->GetMemoryMap(&MemMapSize, MemMap, &LocalMapKey, &DescSize, &DescVersion);
    if (EFI_ERROR(Status)) {
        Print(L"[Error] GetMemoryMap (2nd) failed: %r\n", Status);
        // 재시도 로직을 위해 루프 시작점으로 갈 수 있지만, 여기서는 단순화
        gBS->FreePool(MemMap); // 실패했으므로 gBS 사용 가능
        return Status;
    }

    // 4단계: ExitBootServices 호출
    Status = gBS->ExitBootServices(ImageHandle, LocalMapKey);
    if (EFI_ERROR(Status)) {
        Print(L"[Retry] ExitBootServices failed (%r), retrying...\n", Status);
        // GetMemoryMap이 MapKey를 변경했을 수 있으므로, 전체 프로세스를 다시 시도
        // (MemMapSize는 이전 값을 사용하거나 다시 0으로 설정하고 요청)
        MemMapSize = 0; // 사이즈부터 다시 요청
        Status = gBS->GetMemoryMap(&MemMapSize, NULL, &LocalMapKey, &DescSize, &DescVersion);
        if (Status != EFI_BUFFER_TOO_SMALL) {
            Print(L"[Retry] GetMemoryMap for retry failed: %r\n", Status);
            gBS->FreePool(MemMap); // 실패했으므로 gBS 사용 가능
            return Status;
        }
        goto RetryAllocateAndGetMap; // 할당부터 다시
    }

    // ExitBootServices가 성공한 경우
    ExitBootServicesSucceeded = TRUE;
    if (MapKeyOut) *MapKeyOut = LocalMapKey;
    // Print(L"[OK] ExitBootServices successful.\n"); // UEFI 서비스 종료 후 Print 사용 불가!
    // gBS->FreePool(MemMap); // UEFI 서비스 종료 후 FreePool 사용 불가! MemMap은 OS가 관리

    // MemMap은 ExitBootServices 성공 시 해제하지 않습니다. OS가 이 메모리를 사용합니다.
    // 실패 시에는 위에서 이미 해제되었거나, 이 함수를 빠져나가기 전에 해제됩니다.
    if (!ExitBootServicesSucceeded && MemMap != NULL) {
         gBS->FreePool(MemMap);
    }

    if (ExitBootServicesSucceeded) {
         // 여기서는 어떤 gBS 서비스도 호출하면 안 됩니다.
         // 필요한 정보 (MapKey 등)는 이미 반환 준비가 되었습니다.
    } else {
        Print(L"[FAIL] ExitBootServices failed definitively: %r\n", Status);
    }

    return Status;
}