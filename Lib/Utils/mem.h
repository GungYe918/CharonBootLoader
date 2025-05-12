#pragma once
#include <stddef.h>
#include <Uefi.h>

void *memcpy(void *dest, const void *src, size_t n);
void* memset(void* dst, int val, UINTN n);
UINTN strlen(const char* str);