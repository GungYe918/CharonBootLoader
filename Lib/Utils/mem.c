#include "mem.h"


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

VOID *Kmemcpy(VOID *Dest, const VOID *Src, UINTN Len) {
    UINT8 *d = (UINT8 *)Dest;
    const UINT8 *s = (const UINT8 *)Src;
    while (Len--) *d++ = *s++;
    return Dest;
}