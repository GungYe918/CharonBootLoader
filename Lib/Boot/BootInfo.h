#ifndef _BOOTINFO_H_
#define _BOOTINFO_H_

#include <Uefi.h>

// BootInfo 구조체 정의 (FreeBSD 호환)
typedef struct {
    UINT32 bi_version;               // 구조체 버전
    UINT32 bi_kernelname;            // 커널 파일 이름 포인터
    UINT64 bi_framebuffer_addr;      // 프레임버퍼 주소
    UINT32 bi_framebuffer_width;     // 화면 너비
    UINT32 bi_framebuffer_height;    // 화면 높이
    UINT32 bi_framebuffer_pitch;     // 프레임버퍼 피치
    UINT32 bi_howto;                 // 부팅 옵션
    UINT32 bi_memsize;               // 메모리 크기 (MB)
    UINT64 bi_modulep;               // preloaded_file 리스트 포인터
    UINT64 bi_envp;                  // 환경 변수 포인터
    UINT64 bi_cmdline;               // 커맨드라인 문자열 포인터
    UINT64 bi_kernelbase;            // 커널이 메모리에 로드된 물리 주소
} BootInfo;

// PreloadedFile 구조체 정의
typedef struct preloaded_file {
    const CHAR8 *f_name;             // 파일 이름
    const CHAR8 *f_type;             // "elf kernel", "module" 등
    UINT64 f_addr;                   // 로드된 주소
    UINT64 f_size;                   // 파일 크기
    struct preloaded_file *f_next;   // 다음 파일 포인터
} PreloadedFile;

// BootInfo 구조체 버전
#define BOOTINFO_VERSION    1

// bi_howto에 대한 플래그 정의
#define RB_ASKNAME      0x00000001  // 부팅할 파일 이름 질문
#define RB_SINGLE       0x00000002  // 싱글 유저 모드로 부팅
#define RB_NOSYNC       0x00000004  // 싱크 작업 생략
#define RB_HALT         0x00000008  // 부팅 후 halt
#define RB_VERBOSE      0x00000010  // 상세 메시지 출력
#define RB_QUIET        0x00000020  // 조용한 모드
#define RB_DFLTROOT     0x00000040  // 기본 루트 장치 사용
#define RB_KDB          0x00000080  // 커널 디버거로 진입
#define RB_RDONLY       0x00000100  // 읽기 전용으로 마운트
#define RB_DUMP         0x00000200  // 덤프 요청 후 재부팅
#define RB_MINIROOT     0x00000400  // 미니루트 사용
#define RB_CONFIG       0x00000800  // 구성 추출
#define RB_MULTIPLE     0x00001000  // 여러 consoles 허용
#define RB_MUTE         0x00002000  // 콘솔 출력 음소거
#define RB_SERIAL       0x00004000  // 시리얼 콘솔 사용
#define RB_CDROM        0x00008000  // CD-ROM에서 부팅

#endif /* _BOOTINFO_H_ */