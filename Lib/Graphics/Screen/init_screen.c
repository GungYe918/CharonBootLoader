#include "InitScreen.h"
#include "../../Utils/Print.h"


UINT32 GetPixelColor(UINT32 r, UINT32 g, UINT32 b) {
    EFI_GRAPHICS_PIXEL_FORMAT format = gGop->Mode->Info->PixelFormat;
    UINT32 color = 0;
    
    // 0-255 범위 확인
    r = (r > 255) ? 255 : r;
    g = (g > 255) ? 255 : g;
    b = (b > 255) ? 255 : b;
    
    switch (format) {
        case PixelRedGreenBlueReserved8BitPerColor:
            // RGBA 형식
            UINT32 tmp_b = b;
            UINT32 tmp_r = r;

            b = tmp_r;
            r = tmp_b;
            color = (r) | (g << 8) | (b << 16) | (255 << 24);
            break;
            
        case PixelBlueGreenRedReserved8BitPerColor:
            // BGRA 형식
            

            color = (b) | (g << 8) | (r << 16) | (255 << 24);

            

            break;
            
        case PixelBitMask: {
            // 비트마스크 형식: 각 색상 채널의 마스크와 위치에 따라 색상을 조합
            EFI_PIXEL_BITMASK mask = gGop->Mode->Info->PixelInformation;
            
            // 각 색상 채널의 시프트 위치 계산
            int r_shift = __builtin_ctz(mask.RedMask);
            int g_shift = __builtin_ctz(mask.GreenMask);

            int b_shift = __builtin_ctz(mask.BlueMask);
            int reserved_shift = __builtin_ctz(mask.ReservedMask);
            
            // 각 색상 채널의 최대 값 계산 (비트 수에 따라)
            int r_max = mask.RedMask >> r_shift;
            int g_max = mask.GreenMask >> g_shift;
            int b_max = mask.BlueMask >> b_shift;


            UINT32 scaled_r = (r * r_max + 127) / 255;
            UINT32 scaled_g = (g * g_max + 127) / 255;
            UINT32 scaled_b = (b * b_max + 127) / 255;
            
            // 알파 채널이 있는 경우 최대값으로 설정
            UINT32 scaled_reserved = mask.ReservedMask ? mask.ReservedMask >> reserved_shift : 0;
            
            // 색상 채널 조합
            color = (scaled_r << r_shift)    | 
                    (scaled_g << g_shift)    | 
                    (scaled_b << b_shift)    ;
                   
            if (mask.ReservedMask) {
                color |= (scaled_reserved << reserved_shift);
            }



            break;
        }
            

        default:
            // 지원되지 않는 형식 -> 검정색 반ghls
            color = 0;
            break;
    }
    
    return color;
}

PixelFormat ConvertPixelFormat(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
    switch (fmt) {
        case PixelRedGreenBlueReserved8BitPerColor:
            return kPixelRGBResv8BitPerColor;
        case PixelBlueGreenRedReserved8BitPerColor:
            return kPixelBGRResv8BitPerColor;
        default:
            // 최소한 crash 나지 않게 디폴트
            return kPixelRGBResv8BitPerColor;
    }
}

void FillScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 color) {
    UINT32 *fb = (UINT32 *) (UINTN)gop->Mode->FrameBufferBase;
    UINTN width = gop->Mode->Info->HorizontalResolution;
    UINTN height = gop->Mode->Info->VerticalResolution;
    UINTN pixels = width * height;

    for (UINTN i = 0; i < pixels; i++) {
        fb[i] = color;
    }
}


EFI_STATUS InitScreen(EFI_SYSTEM_TABLE *SystemTable, UINT32 width, UINT32 height, UINT32 color) {
    gST = SystemTable;
    gBS = SystemTable->BootServices;

    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS status = gBS->LocateProtocol(&gopGuid, NULL, (void**)&gGop);

    if (EFI_ERROR(status)) {
        Print(L"GOP not found.\n");
        return status;
    }

    UINTN maxMode = gGop->Mode->MaxMode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN infoSize;

    UINT32 bestMode = 0;
    BOOLEAN found = FALSE;

    for (UINT32 mode = 0; mode < maxMode; ++mode) {
        status = gGop->QueryMode(gGop, mode, &infoSize, &info);

        if (EFI_ERROR(status)) continue;

        if (info->VerticalResolution == height && info->HorizontalResolution == width) {
            found = TRUE;
            bestMode = mode;


            break;
        }
    }

    if (!found) {
        Print(L"Cannot found requested Resolution... Error\n");
        return EFI_UNSUPPORTED;
    }

    status = gGop->SetMode(gGop, bestMode);
    if (EFI_ERROR(status)) {
        Print(L"Failed to set mode... Error\n");
        return status;
    }

    Print(L"Resolution setted. Filling Screen...[color_code : %x]\n", color);


    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *currentInfo = gGop->Mode->Info;
    Print(L"Resolution is [ width : %lu | height : %lu ]\n", currentInfo->HorizontalResolution, currentInfo->VerticalResolution);

    FillScreen(gGop, color);

    return EFI_SUCCESS;
}


