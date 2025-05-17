#include "loadElf.h"


BOOLEAN IsValidElf(Elf64_Ehdr* hdr) {
    return IS_ELF(*hdr) &&
        hdr->e_ident[EI_CLASS] == ELFCLASS64 &&
        hdr->e_ident[EI_DATA]  == ELFDATA2LSB;
}


EFI_STATUS ParseElfHeader(EFI_FILE_PROTOCOL* File, ElfImage* image) {
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

    
    Print(L"ELF Header OK. Entry = 0x%lx\n", hdr->e_entry);

    
    return EFI_SUCCESS;
}

EFI_STATUS LoadElfFile(EFI_HANDLE ImageHandle) {
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

    Status = ParseElfHeader(KernelFile, &image);
    if (EFI_ERROR(Status)) {
        Print(L"ELF header invalid\n");
        return Status;
    }

    return EFI_SUCCESS;
}