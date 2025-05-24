#pragma once
#include <stddef.h>
#include <Uefi.h>

// simple func for normal memory calculation
void *memcpy(void *dest, const void *src, size_t n);
void* memset(void* dst, int val, UINTN n);
UINTN strlen(const char* str);


/**
 * Mem Uitls for UEFI
 */

// UEFI용 메모리 복사 함수
VOID *Kmemcpy(VOID *Dest, const VOID *Src, UINTN Len);

// 특정 범위 메모리를 0으로 초기화
VOID ZeroMem(VOID* Buffer, UINTN Size);

// 메모리 맵 스캔 후 MB단위로 표시
UINT32 ScanMemMap_MB();