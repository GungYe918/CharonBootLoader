#include "Load.h"
#include "../Utils/mem.h"
#include <Guid/FileInfo.h>



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

EFI_STATUS LoadElfFile(EFI_HANDLE ImageHandle, CHAR16* ElfPath) {
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
        ElfPath,
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


// 메모리 영역이 EfiConventionalMemory인지 확인하는 함수
BOOLEAN IsMemoryRegionUsable(EFI_PHYSICAL_ADDRESS Addr, UINTN Pages) {
    EFI_MEMORY_DESCRIPTOR *MemMap = NULL;
    UINTN MemMapSize = 0, MapKey, DescriptorSize;
    UINT32 DescriptorVersion;
    EFI_STATUS Status;

    // 메모리 맵 크기 얻기
    Status = gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    MemMapSize += DescriptorSize * 2;

    Status = gBS->AllocatePool(EfiLoaderData, MemMapSize, (VOID **)&MemMap);
    if (EFI_ERROR(Status)) return FALSE;

    Status = gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        gBS->FreePool(MemMap);
        return FALSE;
    }

    BOOLEAN Usable = FALSE;
    EFI_MEMORY_DESCRIPTOR *Desc = MemMap;
    UINTN EntryCount = MemMapSize / DescriptorSize;

    for (UINTN i = 0; i < EntryCount; i++) {
        EFI_PHYSICAL_ADDRESS Start = Desc->PhysicalStart;
        EFI_PHYSICAL_ADDRESS End = Start + Desc->NumberOfPages * 4096;

        if (Desc->Type == EfiConventionalMemory &&
            Addr >= Start &&
            (Addr + Pages * 4096) <= End) {
            Usable = TRUE;
            break;
        }

        Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Desc + DescriptorSize);
    }

    gBS->FreePool(MemMap);
    return Usable;
}


EFI_STATUS LoadKernel(
    EFI_HANDLE ImageHandle,
    CHAR16 *KernelPath,
    UINT64 *EntryPoint,
    UINT64 *KernelBase,
    UINT64 *KernelSize
) {
    EFI_STATUS Status;

    // ImageProtocol 가져오기
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    Status = gBS->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    );
    if(EFI_ERROR(Status)) {
        Print(L"[Error] Failed to get LoadedImageProtocol: %x\n", Status);
        return Status;
    }


    // simpleFileSystem을 이용한 파일 open
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSys;
    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**)&FileSys
    );
    if(EFI_ERROR(Status)) {
        Print(L"[Error] Failed to get SimpleFileSystemProtocol : %x\n", Status);
        return Status;
    }


    // 루트 폴더 열기
    EFI_FILE_PROTOCOL *Root;
    Status = FileSys->OpenVolume(FileSys, &Root);
    if (EFI_ERROR(Status)) {
        Print(L"[Error] Failed to open Root Volumee : %x\n", Status);
        return Status;
    }


    // 커널 파일 열기
    EFI_FILE_PROTOCOL *KernelFile;
    Status = Root->Open(
        Root, &KernelFile,
        KernelPath, 
        EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) {
        Print(L"[Error] Failed to open KernelFile : %x\n", Status);
    }


    // Elf 헤더 읽기
    Elf64_Ehdr Ehdr;
    UINTN Size = sizeof(Ehdr);
    Status = KernelFile->Read(KernelFile, &Size, &Ehdr);
    if (EFI_ERROR(Status)) {
        Print(L"Corrupted Elf file... : %x\n", Status);
        return EFI_LOAD_ERROR;
    }


    // 커널 진입 주소 지정
    *EntryPoint = Ehdr.e_entry;
    Print(L"The Kernel Entry is : 0x%lx\n", Ehdr.e_entry);


    // 프로그램 헤더 테이블 로딩
    UINTN PHTSize = Ehdr.e_phentsize * Ehdr.e_phnum;
    Elf64_Phdr *Phdrs;
    Status = gBS->AllocatePool(
        EfiLoaderData, PHTSize, (VOID**)&Phdrs
    );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to Allocate Program Header Table : %x\n", Status);
        return Status;
    }


    Status = KernelFile->SetPosition(KernelFile, Ehdr.e_phoff);
    if (EFI_ERROR(Status)) return Status;


    Size = PHTSize;
    Status = KernelFile->Read(KernelFile, &Size, Phdrs);
    if (EFI_ERROR(Status)) return Status;


    // 커널의 낮은 주소와 높은 주소 설정
    UINT64 LowestAddr   = ~0ULL;
    UINT64 HighestAddr  = 0;


    // 로드할 Segment를 메모리에 복사
    for (UINT16 i = 0; i < Ehdr.e_phnum; ++i) {
        Elf64_Phdr *Phdr = &Phdrs[i];
        if (Phdr->p_type != PT_LOAD || Phdr->p_memsz == 0) continue;


        EFI_PHYSICAL_ADDRESS LoadAddr = Phdr->p_paddr;
        UINTN Pages = (Phdr->p_memsz + 0xFFF) / 0x1000;

        if (LoadAddr < LowestAddr) LowestAddr = LoadAddr;
        if (LoadAddr + Phdr->p_memsz > HighestAddr)
            HighestAddr = LoadAddr + Phdr->p_memsz;

        // 메모리 영역 사용 가능성 검사
        if (!IsMemoryRegionUsable(LoadAddr, Pages)) {
            Print(L"[ERROR] Cannot use address 0x%lx for %lu pages\n", LoadAddr, Pages);
            gBS->FreePool(Phdrs);
            return EFI_OUT_OF_RESOURCES;
        }

        // Segment 데이터 읽기
        Status = KernelFile->SetPosition(KernelFile, Phdr->p_offset);
        if (EFI_ERROR(Status)) {
            gBS->FreePool(Phdrs);
            return Status;
        }

        UINTN CopySize = Phdr->p_filesz;
        VOID *TempBuf;
        Status = gBS->AllocatePool(EfiLoaderData, CopySize, &TempBuf);
        if (EFI_ERROR(Status)) {
            gBS->FreePool(Phdrs);
            return Status;
        }

        Status = KernelFile->Read(KernelFile, &CopySize, TempBuf);
        if (EFI_ERROR(Status)) {
            gBS->FreePool(TempBuf);
            gBS->FreePool(Phdrs);
            return Status;
        }

        // memcpy 구현 사용
        Kmemcpy((VOID *)(UINTN)LoadAddr, TempBuf, CopySize);

        // zero-fill
        if (Phdr->p_memsz > Phdr->p_filesz) {
            UINTN Diff = Phdr->p_memsz - Phdr->p_filesz;
            gBS->SetMem((VOID *)(UINTN)(LoadAddr + Phdr->p_filesz), Diff, 0);
        }

        gBS->FreePool(TempBuf);
        Print(L"[OK] Segment copied to 0x%lx (%lu bytes)\n", LoadAddr, Phdr->p_memsz);   
    }



    gBS->FreePool(Phdrs);
    Print(L"[Final] Kernel Loaded at Entry<0x%lx>\n", *EntryPoint);
    


    Print(L"\nELF Header details:\n\n");
    Print(L"  e_entry: 0x%lx\n", Ehdr.e_entry);
    Print(L"  e_phoff: 0x%lx\n", Ehdr.e_phoff);
    Print(L"  e_phnum: %d\n", Ehdr.e_phnum);
    Print(L"  e_phentsize: %d\n", Ehdr.e_phentsize);
    Print(L"  e_type: %d\n", Ehdr.e_type);
    Print(L"  e_machine: %d\n", Ehdr.e_machine);

    // 아키텍처 검증
#ifdef MDE_CPU_X64
    if (Ehdr.e_machine != EM_X86_64) {
        Print(L"Error: Kernel architecture mismatch\n");
        return EFI_INCOMPATIBLE_VERSION;
    }
#elif defined(MDE_CPU_AARCH64)
    if (Ehdr.e_machine != EM_AARCH64) {
        Print(L"Error: Kernel architecture mismatch\n");
        return EFI_INCOMPATIBLE_VERSION;
    }
#endif

    // KernelBase & KernelSize 설정
    if (KernelBase) *KernelBase = LowestAddr;
    if (KernelSize) *KernelSize = HighestAddr - LowestAddr;

    return EFI_SUCCESS;

}

VOID JumpToKernel(UINT64 EntryPoint, BootInfo *BootData) {

    /*  EntryPoint 설정  */
    VOID (*KernelEntryFunc)(BootInfo*) = (VOID (*)(BootInfo*))(UINTN)EntryPoint;

    // 커널로 점프
    KernelEntryFunc(BootData);

    // 여기로 다시 돌아오면 안됨...
    Print(L"Why you came back??? Panic!!!\n\n");
    while (1) {
        UINTN i;
        for (i = 0; i < 100000; ++i) {
            __asm__ __volatile__(""); // 최적화 방지용...
        }
    }
    Print(L"You alive...?");
}

        
       

//메모리 할당 시작

        //EFI_PHYSICAL_ADDRESS OriginalAddr = Phdr->p_vaddr & ~0xFFFULL;
        //UINTN Pages = (Phdr->p_memsz + (Phdr->p_vaddr - OriginalAddr) + 0xFFF) / 0x1000;

        /*
        EFI_PHYSICAL_ADDRESS LoadAddr = 0;
        Status = gBS->AllocatePages(
            AllocateAnyPages,
            EfiLoaderData,
            Pages,
            &LoadAddr
        );
        if (EFI_ERROR(Status)) {
            Print(L"Failed to Allocate Pages : %r\n", Status);
            return Status;
        }
        */

        // for Debuging...
        //Print(L"Allocate Memory At : 0x%lx | Virtual Addr : 0x%lx\n", LoadAddr, Phdr->p_vaddr);

        //VOID *Target = (VOID *)(UINTN)(LoadAddr + (Phdr->p_vaddr - LoadAddr));


        /*

        // 커널 파일에서 Segment 데이터 읽기
        Status = KernelFile->SetPosition(KernelFile, Phdr->p_offset);
        if (EFI_ERROR(Status)) return Status;



        UINTN CopySize = Phdr->p_filesz;
        Status = KernelFile->Read(
            KernelFile, &CopySize, 
            Target
        );
        if (EFI_ERROR(Status)) return Status;


        UINT64 Diff = Phdr->p_memsz - Phdr->p_filesz;
        if (Diff > 0) {
            gBS->SetMem(
                (VOID *)((UINTN)Target + Phdr->p_filesz), 
                Diff, 0
            );
        }
        */