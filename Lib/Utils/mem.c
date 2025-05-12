#include "mem.h"

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