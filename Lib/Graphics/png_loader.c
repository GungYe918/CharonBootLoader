#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <Protocol/LoadedImage.h>
#include "lodepng.h"
#include "png_loader.h"

extern EFI_BOOT_SERVICES *gBS;
extern EFI_SYSTEM_TABLE *gST;

static void PrintStrr(CHAR16 *str) {
    gST->ConOut->OutputString(gST->ConOut, str);
}

// 16진수 출력 함수 (최대 64비트)
static void PrintHex(UINT8 byte) {
    CHAR16 hex[3];
    UINT8 high = (byte >> 4) & 0xF;
    UINT8 low  = byte & 0xF;

    hex[0] = (high < 10) ? (CHAR16)(L'0' + high) : (CHAR16)(L'A' + high - 10);
    hex[1] = (low  < 10) ? (CHAR16)(L'0' + low)  : (CHAR16)(L'A' + low  - 10);
    hex[2] = L'\0';

    gST->ConOut->OutputString(gST->ConOut, hex);
}

void PrintUIntt(EFI_SYSTEM_TABLE *gST, UINTN value) {
    CHAR16 buf[20];
    int i = 19;
    buf[i--] = L'\0';

    if (value == 0) {
        buf[i--] = L'0';
    } else {
        while (value > 0 && i >= 0) {
            buf[i--] = L'0' + (value % 10);
            value /= 10;
        }
    }

    gST->ConOut->OutputString(gST->ConOut, &buf[i + 1]);
}


bool LoadFileToBuffer(EFI_HANDLE ImageHandle, CHAR16 *filename, UINT8 **buffer, UINTN *size) {
    PrintStrr(L"[LoadFileToBuffer] Start\r\n");

    EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_FILE_PROTOCOL *root, *file;
    EFI_GUID loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID sfspGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID infoGuid = EFI_FILE_INFO_ID;

    EFI_STATUS status = gBS->HandleProtocol(ImageHandle, &loadedImageGuid, (void**)&loadedImage);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] HandleProtocol LoadedImage failed\r\n");
        return false;
    }

    status = gBS->HandleProtocol(loadedImage->DeviceHandle, &sfspGuid, (void**)&fs);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] HandleProtocol SFS failed\r\n");
        return false;
    }

    status = fs->OpenVolume(fs, &root);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] OpenVolume failed\r\n");
        return false;
    }

    status = root->Open(root, &file, filename, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] File open failed\r\n");
        return false;
    }

    // 파일 정보 크기 얻기
    UINTN infoSize = 0;
    status = file->GetInfo(file, &infoGuid, &infoSize, NULL);
    if (status != EFI_BUFFER_TOO_SMALL) {
        PrintStrr(L"[Error] GetInfo (1) failed\r\n");
        return false;
    }

    // 파일 정보를 위한 메모리 할당
    EFI_FILE_INFO *info = NULL;
    status = gBS->AllocatePool(EfiLoaderData, infoSize, (void**)&info);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] AllocatePool for info failed\r\n");
        return false;
    }

    // 파일 정보 얻기
    status = file->GetInfo(file, &infoGuid, &infoSize, info);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] GetInfo (2) failed\r\n");
        gBS->FreePool(info);
        return false;
    }

    // 파일 크기 확인 및 출력
    *size = info->FileSize;
    PrintStrr(L"[LoadFileToBuffer] File size: ");
    PrintUIntt(gST, *size);  // PrintHex 대신 PrintUIntt 사용하여 읽기 쉽게 표시
    PrintStrr(L"\r\n");

    // 파일 데이터를 위한 메모리 할당
    status = gBS->AllocatePool(EfiLoaderData, *size, (void**)buffer);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] AllocatePool for data failed\r\n");
        gBS->FreePool(info);
        return false;
    }

    // 파일 읽기
    UINTN readSize = *size;  // 원래 크기 보존
    status = file->Read(file, &readSize, *buffer);
    if (EFI_ERROR(status)) {
        PrintStrr(L"[Error] File read failed\r\n");
        gBS->FreePool(info);
        gBS->FreePool(*buffer);
        return false;
    }

    // 읽은 크기와 요청한 크기 비교
    if (readSize != *size) {
        PrintStrr(L"[Warning] Read size (");
        PrintUIntt(gST, readSize);
        PrintStrr(L") differs from file size (");
        PrintUIntt(gST, *size);
        PrintStrr(L")\r\n");
        
        // 파일을 처음부터 다시 읽기
        status = file->SetPosition(file, 0);
        if (EFI_ERROR(status)) {
            PrintStrr(L"[Error] SetPosition failed\r\n");
            gBS->FreePool(info);
            gBS->FreePool(*buffer);
            return false;
        }
        
        // 전체 파일을 청크로 나누어 읽기
        UINTN totalRead = 0;
        while (totalRead < *size) {
            UINTN bytesToRead = *size - totalRead;
            status = file->Read(file, &bytesToRead, *buffer + totalRead);
            if (EFI_ERROR(status) || bytesToRead == 0) {
                PrintStrr(L"[Error] Chunked read failed\r\n");
                gBS->FreePool(info);
                gBS->FreePool(*buffer);
                return false;
            }
            totalRead += bytesToRead;
        }
    }

    PrintStrr(L"[LoadFileToBuffer] Read success, final size: ");
    PrintUIntt(gST, *size);
    PrintStrr(L"\r\n");

    // 메모리 해제 및 파일 닫기
    gBS->FreePool(info);
    file->Close(file);
    return true;
}




bool LoadPngFromDisk(EFI_HANDLE ImageHandle, CHAR16 *filename, UINT8 **rgba_out, UINT32 *width, UINT32 *height) {
    gST->ConOut->OutputString(gST->ConOut, L"[LoadPngFromDisk] Start\r\n");

    UINT8 *png_data = NULL;
    UINTN png_size;
    if (!LoadFileToBuffer(ImageHandle, filename, &png_data, &png_size)) {
        gST->ConOut->OutputString(gST->ConOut, L"[Error] LoadFileToBuffer failed\r\n");
        return false;
    }

    // PNG 시그니처 검증 (8바이트)
    if (png_size < 8 ||
        png_data[0] != 0x89 || png_data[1] != 0x50 || png_data[2] != 0x4E || png_data[3] != 0x47 ||
        png_data[4] != 0x0D || png_data[5] != 0x0A || png_data[6] != 0x1A || png_data[7] != 0x0A) {
        PrintStrr(L"[Error] Invalid PNG signature\r\n");
        gBS->FreePool(png_data);
        return false;
    }

    PrintStrr(L"[Header] PNG magic: ");
    for (int i = 0; i < 8; i++) {
        PrintHex(png_data[i]);
        PrintStrr(L" ");
    }
    PrintStrr(L"\r\n");

    // IHDR 청크 확인 (8 + 12 = 20바이트)
    if (png_size < 24) {
        PrintStrr(L"[Error] PNG file too small to contain IHDR chunk\r\n");
        gBS->FreePool(png_data);
        return false;
    }

    // IHDR 청크 길이는 바이트 8-11에 빅 엔디안으로 저장됨
    UINT32 ihdr_length = (png_data[8] << 24) | (png_data[9] << 16) | (png_data[10] << 8) | png_data[11];
    
    // IHDR 청크 타입 확인 (바이트 12-15는 "IHDR"이어야 함)
    if (png_data[12] != 'I' || png_data[13] != 'H' || png_data[14] != 'D' || png_data[15] != 'R') {
        PrintStrr(L"[Error] IHDR chunk not found\r\n");
        gBS->FreePool(png_data);
        return false;
    }

    PrintStrr(L"[IHDR] Length: ");
    PrintUIntt(gST, ihdr_length);
    PrintStrr(L"\r\n");

    // IHDR 청크의 표준 길이는 13바이트
    if (ihdr_length != 13) {
        PrintStrr(L"[Error] Invalid IHDR chunk length\r\n");
        gBS->FreePool(png_data);
        return false;
    }

    // PNG 기본 정보 출력 (디버깅용)
    UINT32 img_width = (png_data[16] << 24) | (png_data[17] << 16) | (png_data[18] << 8) | png_data[19];
    UINT32 img_height = (png_data[20] << 24) | (png_data[21] << 16) | (png_data[22] << 8) | png_data[23];
    UINT8 bit_depth = png_data[24];
    UINT8 color_type = png_data[25];
    UINT8 compression = png_data[26];
    UINT8 filter = png_data[27];
    UINT8 interlace = png_data[28];

    PrintStrr(L"[PNG Info] Width: ");
    PrintUIntt(gST, img_width);
    PrintStrr(L", Height: ");
    PrintUIntt(gST, img_height);
    PrintStrr(L", Bit Depth: ");
    PrintUIntt(gST, bit_depth);
    PrintStrr(L", Color Type: ");
    PrintUIntt(gST, color_type);
    PrintStrr(L", Compression: ");
    PrintUIntt(gST, compression);
    PrintStrr(L", Filter: ");
    PrintUIntt(gST, filter);
    PrintStrr(L", Interlace: ");
    PrintUIntt(gST, interlace);
    PrintStrr(L"\r\n");

    // lodepng를 사용하여 디코딩
    unsigned w, h;
    unsigned char *image = NULL;
    
    PrintStrr(L"[lodepng] Decoding PNG...\r\n");
    unsigned err = lodepng_decode32(&image, &w, &h, png_data, png_size);

    // 원본 PNG 데이터는 더 이상 필요 없음
    gBS->FreePool(png_data);

    if (err) {
        PrintStrr(L"[Error] decode32 error: ");
        PrintUIntt(gST, err);
        PrintStrr(L" (");
        
        // lodepng 에러 코드에 대한 설명 추가
        switch (err) {
            case 1: PrintStrr(L"file open failure"); break;
            case 28: PrintStrr(L"PNG file too small"); break;
            case 29: PrintStrr(L"invalid signature"); break;
            case 30: PrintStrr(L"corrupt file structure"); break;
            case 31: PrintStrr(L"corrupt chunk structure"); break;
            case 57: PrintStrr(L"invalid chunk type"); break;
            case 58: PrintStrr(L"invalid IHDR"); break;
            case 59: PrintStrr(L"invalid compression"); break;
            case 60: PrintStrr(L"invalid filter"); break;
            case 61: PrintStrr(L"invalid interlace"); break;
            case 62: PrintStrr(L"invalid color type"); break;
            case 63: PrintStrr(L"invalid bit depth"); break;
            case 64: PrintStrr(L"invalid ICC profile"); break;
            default: PrintStrr(L"unknown error"); break;
        }
        
        PrintStrr(L")\r\n");
        
        // 이미지 데이터가 할당되었으면 해제
        if (image != NULL) {
            gBS->FreePool(image);
        }
        return false;
    }

    *rgba_out = image;
    *width = w;
    *height = h;

    PrintStrr(L"[lodepng] Successfully decoded image: ");
    PrintUIntt(gST, w);
    PrintStrr(L"x");
    PrintUIntt(gST, h);
    PrintStrr(L"\r\n");

    PrintStrr(L"[LoadPngFromDisk] Success\r\n");
    return true;
}