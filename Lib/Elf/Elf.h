#ifndef _ELF_H_
#define _ELF_H_

#include <Uefi.h>

// ELF 파일 형식 정의

// ELF 헤더 식별자
#define EI_NIDENT       16

// e_ident[] 인덱스
#define EI_MAG0         0       // 파일 ID
#define EI_MAG1         1       // 파일 ID
#define EI_MAG2         2       // 파일 ID
#define EI_MAG3         3       // 파일 ID
#define EI_CLASS        4       // 파일 클래스
#define EI_DATA         5       // 데이터 인코딩
#define EI_VERSION      6       // 파일 버전
#define EI_OSABI        7       // OS/ABI 식별
#define EI_ABIVERSION   8       // ABI 버전
#define EI_PAD          9       // 패딩 시작

// ELF 매직 번호
#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

// e_ident[EI_CLASS]
#define ELFCLASSNONE    0       // 잘못된 클래스
#define ELFCLASS32      1       // 32비트 오브젝트
#define ELFCLASS64      2       // 64비트 오브젝트

// e_ident[EI_DATA]
#define ELFDATANONE     0       // 잘못된 데이터 인코딩
#define ELFDATA2LSB     1       // 2의 보수, 리틀 엔디안
#define ELFDATA2MSB     2       // 2의 보수, 빅 엔디안

// e_type
#define ET_NONE         0       // 파일 타입 없음
#define ET_REL          1       // 재배치 가능 파일
#define ET_EXEC         2       // 실행 파일
#define ET_DYN          3       // 공유 오브젝트 파일
#define ET_CORE         4       // 코어 파일

// e_machine
#define EM_X86_64       62      // AMD x86-64
#define EM_AARCH64      183     // ARM AARCH64

#pragma pack(push, 1)
// ELF 64비트 헤더
typedef struct {
    UINT8   e_ident[EI_NIDENT];  // ELF 식별자
    UINT16  e_type;              // 오브젝트 파일 타입
    UINT16  e_machine;           // 아키텍처
    UINT32  e_version;           // 오브젝트 파일 버전
    UINT64  e_entry;             // 가상 진입점 주소
    UINT64  e_phoff;             // 프로그램 헤더 테이블의 오프셋
    UINT64  e_shoff;             // 섹션 헤더 테이블의 오프셋
    UINT32  e_flags;             // 프로세서 특정 플래그
    UINT16  e_ehsize;            // ELF 헤더 크기
    UINT16  e_phentsize;         // 프로그램 헤더 엔트리 크기
    UINT16  e_phnum;             // 프로그램 헤더 엔트리 수
    UINT16  e_shentsize;         // 섹션 헤더 엔트리 크기
    UINT16  e_shnum;             // 섹션 헤더 엔트리 수
    UINT16  e_shstrndx;          // 섹션 이름 스트링 테이블 인덱스
} Elf64_Ehdr;


// ELF 64비트 프로그램 헤더
typedef struct {
    UINT32  p_type;              // 세그먼트 타입
    UINT32  p_flags;             // 세그먼트 플래그
    UINT64  p_offset;            // 파일 내 세그먼트 오프셋
    UINT64  p_vaddr;             // 가상 주소
    UINT64  p_paddr;             // 물리적 주소 (사용되지 않음)
    UINT64  p_filesz;            // 파일 내 세그먼트 크기
    UINT64  p_memsz;             // 메모리 내 세그먼트 크기
    UINT64  p_align;             // 세그먼트 정렬
} Elf64_Phdr;

#pragma pack(pop)

// 프로그램 헤더 타입
#define PT_NULL         0       // 사용되지 않음
#define PT_LOAD         1       // 로드 가능 세그먼트
#define PT_DYNAMIC      2       // 동적 링킹 정보
#define PT_INTERP       3       // 인터프리터 정보
#define PT_NOTE         4       // 보조 정보
#define PT_SHLIB        5       // 예약됨
#define PT_PHDR         6       // 프로그램 헤더 테이블

// 프로그램 헤더 플래그
#define PF_X            0x1     // 실행 가능
#define PF_W            0x2     // 쓰기 가능
#define PF_R            0x4     // 읽기 가능

// ELF 파일이 유효한지 확인하는 매크로
#define IS_ELF(ehdr) ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
                      (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
                      (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
                      (ehdr).e_ident[EI_MAG3] == ELFMAG3)

#endif /* _ELF_H_ */