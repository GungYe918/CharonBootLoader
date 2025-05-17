#include "Print.h"
#include <stddef.h>

extern EFI_SYSTEM_TABLE *gST;


void PrintStr(EFI_SYSTEM_TABLE *SystemTable, CHAR16 *str) {
    CHAR16 fixed[1024];
    int j = 0;

    for (int i = 0; str[i] != L'\0' && j < 1022; ++i) {
        if (str[i] == L'\n') {
            fixed[j++] = L'\r';
            fixed[j++] = L'\n';
        }
        
        else if (str[i] == L'\r') {
            fixed[j++] = L'\r';
        }

        else if (str[i] == L'\t') {
            for (int t = 0; t < 4 && t < 1023; ++t) {
                fixed[j++] = L' ';
            }
        }

        else if (str[i] == '\b') {
            if (j > 0) (j--);
        }

        else {
            fixed[j++] = str[i];
        }
    }

    fixed[j] = L'\0';
    SystemTable->ConOut->OutputString(SystemTable->ConOut, fixed);
}

// 숫자 -> 문자열 변환 (10진수)
void UIntToDecimalStr(UINTN value, CHAR16 *buf) {
    CHAR16 tmp[32];
    int i = 0;
    if (value == 0) {
        buf[0] = L'0';
        buf[1] = L'\0';
        return;
    }

    while (value > 0 && i < 31) {
        tmp[i++] = L'0' + (value % 10);
        value /= 10;
    }

    // 역순으로 복사
    for (int j = 0; j < i; j++) {
        buf[j] = tmp[i - j - 1];
    }
    buf[i] = L'\0';
}

// 숫자 -> 문자열 변환 (16진수)
void UIntToHexStr(UINTN value, CHAR16 *buf) {
    CHAR16 hexChars[] = L"0123456789ABCDEF";
    CHAR16 tmp[32];
    int i = 0;
    if (value == 0) {
        buf[0] = L'0';
        buf[1] = L'\0';
        return;
    }

    while (value > 0 && i < 31) {
        tmp[i++] = hexChars[value % 16];
        value /= 16;
    }

    // 역순 복사
    for (int j = 0; j < i; j++) {
        buf[j] = tmp[i - j - 1];
    }
    buf[i] = L'\0';
}


void UInt64ToHexStr(UINT64 value, CHAR16 *buf) {
    CHAR16 hexChars[] = L"0123456789ABCDEF";
    CHAR16 tmp[32];
    int i = 0;
    if (value == 0) {
        buf[0] = L'0';
        buf[1] = L'\0';
        return;
    }

    while (value > 0 && i < 31) {
        tmp[i++] = hexChars[value % 16];
        value /= 16;
    }

    // 역순 복사
    for (int j = 0; j < i; j++) {
        buf[j] = tmp[i - j - 1];
    }
    buf[i] = L'\0';
}

void Print(CHAR16 *fmt, ...) {
    CHAR16 buf[512];
    UINTN bi = 0;

    va_list args;
    va_start(args, fmt);

    for (UINTN i = 0; fmt[i] != L'\0' && bi < 511; i++) {
        if (fmt[i] == L'%' && fmt[i + 1] != L'\0') {
            i++;
            if (fmt[i] == L'd') {
                UINTN val = va_arg(args, UINTN);
                CHAR16 numbuf[32];
                UIntToDecimalStr(val, numbuf);
                for (UINTN j = 0; numbuf[j] != L'\0' && bi < 511; j++) {
                    buf[bi++] = numbuf[j];
                }
            } else if (fmt[i] == L'x') {
                UINTN val = va_arg(args, UINTN);
                CHAR16 numbuf[32];
                UIntToHexStr(val, numbuf);
                for (UINTN j = 0; numbuf[j] != L'\0' && bi < 511; j++) {
                    buf[bi++] = numbuf[j];
                }
            } else if (fmt[i] == L'l') {
                if (fmt[i+1] == L'x') {
                    i++;    // x부분은 건너뛰기
                    UINT64 val = va_arg(args, UINT64);
                    CHAR16 numbuf[32];
                    UInt64ToHexStr(val, numbuf);
                    for (UINTN j = 0; numbuf[j] != L'\0' && bi < 511; j++) {
                        buf[bi++] = numbuf[j];
                    }
                } else if (fmt[i+1] == L'u') {
                    i++;
                    UINT64 val = va_arg(args, UINT64);
                    CHAR16 numbuf[32];
                    UIntToDecimalStr((UINTN)val, numbuf);
                    for (UINTN j = 0; numbuf[j] != L'\0' && bi < 511; j++) {
                        buf[bi++] = numbuf[j];
                    }
                }
            } else if (fmt[i] == L's') {
                CHAR16* str = va_arg(args, CHAR16*);
                for (UINTN j = 0; str[j] != L'\0' && bi < 511; j++) {
                    buf[bi++] = str[j];
                }
            } else {
                // % 외 문자는 그대로 출력
                buf[bi++] = L'%';
                buf[bi++] = fmt[i];
            }
        } else {
            buf[bi++] = fmt[i];
        }
    }

    buf[bi] = L'\0';
    va_end(args);

    PrintStr(gST, buf);
}