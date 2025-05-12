#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <Protocol/LoadedImage.h>
//#include "Lib/Graphics/png_loader.h"
#include "Lib/Elf/Elf.h"
#include "Lib/Utils/Print.h"
#include "Lib/Graphics/Picture.h"
#include "Lib/Utils/mem.h"

EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;

__attribute__((section(".reloc")))
const unsigned char dummy_reloc[1] = { 0 };

void FillScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 color) {
    UINT32 *fb = (UINT32 *) (UINTN)gop->Mode->FrameBufferBase;
    UINTN width = gop->Mode->Info->HorizontalResolution;
    UINTN height = gop->Mode->Info->VerticalResolution;
    UINTN pixels = width * height;

    for (UINTN i = 0; i < pixels; i++) {
        fb[i] = color;
    }
}

// Elf 파일 파싱
#define ELF_BUFFER_SIZE 0x1000  // 4KB만 읽기

typedef struct {
    VOID* Buffer;
    UINTN Size;
} ElfImage;

BOOLEAN IsValidElf(Elf64_Ehdr* hdr) {
    return IS_ELF(*hdr) &&
        hdr->e_ident[EI_CLASS] == ELFCLASS64 &&
        hdr->e_ident[EI_DATA]  == ELFDATA2LSB;
}

EFI_STATUS LoadElfHeader(EFI_FILE_PROTOCOL* File, ElfImage* image) {
    EFI_STATUS Status;
    image->Size = ELF_BUFFER_SIZE;

    Status = File->Read(File, &image->Size, image->Buffer);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to read ELF file!\n");
        return Status;
    }

    Elf64_Ehdr* hdr = (Elf64_Ehdr*) image->Buffer;
    if (!IsValidElf(hdr)) {
        Print(L"Invalid ELF file format\n");
        return EFI_LOAD_ERROR;
    }

    
    Print(L"ELF Header OK. Entry = 0x%x\n", (UINTN)hdr->e_entry);
    return EFI_SUCCESS;
}

EFI_STATUS LoadKernelElf(EFI_HANDLE ImageHandle) {
    EFI_STATUS Status;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_FILE_PROTOCOL* Root;
    EFI_FILE_PROTOCOL* KernelFile;

    // 1. Loaded Image에서 Device Handle 가져오기
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    Status = gBS->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get LoadedImage\n");
        return Status;
    }

    // 2. Simple FileSystem Protocol 열기
    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**)&FileSystem
    );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get FileSystem\n");
        return Status;
    }

    // 3. ESP 루트 디렉토리 열기
    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open root volume\n");
        return Status;
    }

    // 4. kernel.elf 파일 열기
    Status = Root->Open(
        Root,
        &KernelFile,
        L"\\kernel.elf",
        EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel.elf\n");
        return Status;
    }

    // 5. ELF 헤더 버퍼 준비
    ElfImage image;
    UINT8 buffer[ELF_BUFFER_SIZE];
    image.Buffer = buffer;
    image.Size = ELF_BUFFER_SIZE;

    KernelFile->SetPosition(KernelFile, 0); // 파일 처음으로 이동

    Status = LoadElfHeader(KernelFile, &image);
    if (EFI_ERROR(Status)) {
        Print(L"ELF header invalid\n");
        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    gST = SystemTable;
    gBS = SystemTable->BootServices;

    gST->ConOut->OutputString(gST->ConOut, L"Hello, World!!\r\n");

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS status = gBS->LocateProtocol(&gopGuid, NULL, (void**)&gop);

    if (EFI_ERROR(status)) {
        gST->ConOut->OutputString(gST->ConOut, L"GOP not found.\r\n");
    } else {
        gST->ConOut->OutputString(gST->ConOut, L"Resolution: ");
        // 간단한 숫자 출력 (예전 PrintUInt 함수 재사용)
        CHAR16 buf[32];
        buf[0] = 0;
        gST->ConOut->OutputString(gST->ConOut, L"800x600\r\n");

        // 파란색으로 화면 초기화 (0xFF0000FF: Alpha Red Green Blue)
        FillScreen(gop, 0xFF0000FF);  // BGRA: 파란색

        
    }

    /* TODO: fix error... maybe...
    UINT8 *image;
    UINT32 w, h;
    if (LoadPngFromDisk(ImageHandle, L"\\logo.png", &image, &w, &h)) {
        DrawImage(gop, image, w, h, 100, 100);
    } else {
        gST->ConOut->OutputString(gST->ConOut, L"PNG load failed\r\n");
    }*/

    Print(L"Hello, world! 1 + 1 = %d\n", 2);
    Print(L"1\n2\t3\t4\t5\t6\n\n");

    LoadKernelElf(ImageHandle);


    while (1);
    return EFI_SUCCESS;
}
