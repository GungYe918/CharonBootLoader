#include "mem.h"
#include "Print.h"

/**
 * 외부 라이브러리와의 호환성을 위한 대체 매모리 할당 함수
 */
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
  }
  
void* memset(void* dst, int val, UINTN n) {
  UINT8* d = dst;
  while (n--) *d++ = (UINT8)val;
  return dst;
}
  
  UINTN strlen(const char* str) {
    UINTN len = 0;
    while (str[len]) len++;
    return len;
  }


  

/**
 * 커널 로드와 관련된 함수
 */

extern EFI_BOOT_SERVICES *gBS;

VOID *Kmemcpy(VOID *Dest, const VOID *Src, UINTN Len) {
    UINT8 *d = (UINT8 *)Dest;
    const UINT8 *s = (const UINT8 *)Src;
    while (Len--) *d++ = *s++;
    return Dest;
}

VOID ZeroMem(VOID* Buffer, UINTN Size) {
   if (Buffer == NULL || Size == 0) return;

   // 포인터 -> 바이트 단위 casting ==> 0으로 채움(zero fill)
   UINT8* Ptr = (UINT8*)Buffer;
   for (UINT8 i = 0; i < Size; ++i) {
     Ptr[i] = 0;
   }

}


UINT32 ScanMemMap_MB() {
  EFI_STATUS Status;
  EFI_MEMORY_DESCRIPTOR *MemMap = NULL;
  UINTN MemMapSize = 0;
  UINTN MapKey;
  UINTN DescSize;
  UINT32 DescVersion;
  UINT32 TotalMemPages = 0;

  // 메모리 맵 크기만 가져오기
  Status = gBS->GetMemoryMap(
    &MemMapSize, NULL,
    &MapKey, &DescSize, &DescVersion
  );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"Unexpected GetMemoryMap error (not BUFFER_TOO_SMALL)!! | %r\n", Status);
    return 0;
  }

  // 여유분의 버퍼 확보
  MemMapSize += 2*DescSize;

  Status = gBS->AllocatePool(EfiLoaderData, MemMapSize, (VOID**)&MemMap);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to execute AllocatePool | %r\n", Status);
    return 0;
  }

  Status = gBS->GetMemoryMap(
    &MemMapSize,
    MemMap,
    &MapKey,
    &DescSize,
    &DescVersion
  );
  if(EFI_ERROR(Status)) {
    Print(L"Failed to GetMemoryMap | %r\n", Status);
    gBS->FreePool(MemMap);
    return 0;
  }

  // 메모리 맵 순회 & EfiConventionalMemory 값 합산
  UINTN EntryCount = MemMapSize / DescSize;
  for (UINTN i = 0; i < EntryCount; ++i) {
    EFI_MEMORY_DESCRIPTOR* Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + (i * DescSize));

    if (Desc->Type == EfiConventionalMemory) {
      TotalMemPages += (UINT32)(Desc->NumberOfPages);
    }
  }

  gBS->FreePool(MemMap);

  
  
  // 페이지 수(4KB) -> MB로 변환
  UINT64 TotalBytes = (UINT64)TotalMemPages * 4096;
  UINT32 TotalMB = (UINT32)(TotalBytes / (1024 * 1024));
  return TotalMB;
}