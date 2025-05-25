#include "Print.h"
#include <stddef.h>

extern EFI_SYSTEM_TABLE *gST;

typedef struct {
    EFI_STATUS Code;
    CHAR16 *Message;
} StatusCodeMap;

StatusCodeMap EfiStatusMessages[] = {
    // 정상 종료
    { EFI_SUCCESS,                L"Success" },

    // 오류 코드 (Error Codes)
    { EFI_LOAD_ERROR,             L"Load Error" },
    { EFI_INVALID_PARAMETER,      L"Invalid Parameter" },
    { EFI_UNSUPPORTED,            L"Unsupported" },
    { EFI_BAD_BUFFER_SIZE,        L"Bad Buffer Size" },
    { EFI_BUFFER_TOO_SMALL,       L"Buffer Too Small" },
    { EFI_NOT_READY,              L"Not Ready" },
    { EFI_DEVICE_ERROR,           L"Device Error" },
    { EFI_WRITE_PROTECTED,        L"Write Protected" },
    { EFI_OUT_OF_RESOURCES,       L"Out of Resources" },
    { EFI_VOLUME_CORRUPTED,       L"Volume Corrupted" },
    { EFI_VOLUME_FULL,            L"Volume Full" },
    { EFI_NO_MEDIA,               L"No Media" },
    { EFI_MEDIA_CHANGED,          L"Media Changed" },
    { EFI_NOT_FOUND,              L"Not Found" },
    { EFI_ACCESS_DENIED,          L"Access Denied" },
    { EFI_NO_RESPONSE,            L"No Response" },
    { EFI_NO_MAPPING,             L"No Mapping" },
    { EFI_TIMEOUT,                L"Timeout" },
    { EFI_NOT_STARTED,            L"Not Started" },
    { EFI_ALREADY_STARTED,        L"Already Started" },
    { EFI_ABORTED,                L"Aborted" },
    { EFI_ICMP_ERROR,             L"ICMP Error" },
    { EFI_TFTP_ERROR,             L"TFTP Error" },
    { EFI_PROTOCOL_ERROR,         L"Protocol Error" },
    { EFI_INCOMPATIBLE_VERSION,   L"Incompatible Version" },
    { EFI_SECURITY_VIOLATION,     L"Security Violation" },
    { EFI_CRC_ERROR,              L"CRC Error" },
    { EFI_END_OF_MEDIA,           L"End of Media" },
    { EFI_END_OF_FILE,            L"End of File" },
    { EFI_INVALID_LANGUAGE,       L"Invalid Language" },
    { EFI_COMPROMISED_DATA,       L"Compromised Data" },
    { EFI_IP_ADDRESS_CONFLICT,    L"IP Address Conflict" },
    { EFI_HTTP_ERROR,             L"HTTP Error" },

    // 경고 코드 (Warning Codes)
    { EFI_WARN_UNKNOWN_GLYPH,     L"Warning: Unknown Glyph" },
    { EFI_WARN_DELETE_FAILURE,    L"Warning: Delete Failure" },
    { EFI_WARN_WRITE_FAILURE,     L"Warning: Write Failure" },
    { EFI_WARN_BUFFER_TOO_SMALL,  L"Warning: Buffer Too Small" },
    { EFI_WARN_STALE_DATA,        L"Warning: Stale Data" },
    { EFI_WARN_FILE_SYSTEM,       L"Warning: File System" },
    { EFI_WARN_RESET_REQUIRED,    L"Warning: Reset Required" },

    // 마지막 종료 항목
    { 0, NULL }
};



CHAR16* StatusToStr(EFI_STATUS code) {
    for (int i = 0; EfiStatusMessages[i].Message != NULL; i++) {
        if (EfiStatusMessages[i].Code == code)
            return EfiStatusMessages[i].Message;
    }
    return L"Unknown Status";
}

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


void UInt64ToDecimalStr(UINT64 value, CHAR16 *buf) {
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

            // 패딩 숫자 추출 (예: %02x)
            UINTN pad_width = 0;
            if (fmt[i] == L'0') {
                i++;
                while (fmt[i] >= L'0' && fmt[i] <= L'9') {
                    pad_width = pad_width * 10 + (fmt[i] - L'0');
                    i++;
                }
            }

            CHAR16 numbuf[32] = {0};

            if (fmt[i] == L'd' || fmt[i] == L'u') {
                UINTN val = va_arg(args, UINTN);
                UIntToDecimalStr(val, numbuf);
            } else if (fmt[i] == L'x') {
                UINTN val = va_arg(args, UINTN);
                UIntToHexStr(val, numbuf);
            } else if (fmt[i] == L'l' && (fmt[i+1] == L'u' || fmt[i+1] == L'x')) {
                UINT64 val = va_arg(args, UINT64);
                if (fmt[i+1] == L'u') {
                    UInt64ToDecimalStr(val, numbuf);
                } else {
                    UInt64ToHexStr(val, numbuf);
                }
                i++;
            } else if (fmt[i] == L's') {
                CHAR16* str = va_arg(args, CHAR16*);
                for (UINTN j = 0; str[j] != L'\0' && bi < 511; j++) {
                    buf[bi++] = str[j];
                }
                continue;
            } else if (fmt[i] == L'r') {
                EFI_STATUS val = va_arg(args, EFI_STATUS);
                CHAR16 *msg = StatusToStr(val);
                for (UINTN j = 0; msg[j] != L'\0' && bi < 511; j++) {
                    buf[bi++] = msg[j];
                }
                continue;
            } else {
                buf[bi++] = L'%';
                buf[bi++] = fmt[i];
                continue;
            }

            // 패딩 적용
            UINTN len = 0; while (numbuf[len] != L'\0') len++;
            for (UINTN p = len; p < pad_width && bi < 511; p++) buf[bi++] = L'0';
            for (UINTN j = 0; numbuf[j] != L'\0' && bi < 511; j++) buf[bi++] = numbuf[j];
        } else {
            buf[bi++] = fmt[i];
        }
    }

    buf[bi] = L'\0';
    va_end(args);

    PrintStr(gST, buf);
}
