#pragma once
#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

#include "../Elf/Elf.h"
#include "../Utils/Print.h"

extern EFI_BOOT_SERVICES *gBS;

#define ELF_BUFFER_SIZE 0x1000  // 4KB만 읽기


typedef struct {
    VOID* Buffer;
    UINTN Size;
} ElfImage;

BOOLEAN IsValidElf(Elf64_Ehdr* hdr);
EFI_STATUS ParseElfHeader(EFI_FILE_PROTOCOL* File, ElfImage* image);
EFI_STATUS LoadElfFile(EFI_HANDLE ImageHandle, CHAR16* ELfPath);

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle, CHAR16 *KernelPath, UINT64 *EntryPoint);


