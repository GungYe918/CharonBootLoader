# CharonOS Bootloader in UEFI!

이 프로젝트는 운영체제 CharonOS를 부팅하기 위한 부트로더를 개발하는 것을 목적으로 합니다.  
EDK2의 라이브러리나 코드에 의존하지 않으며, 표준 UEFI 헤더만을 사용하여 작성되었습니다.

---

## 📁 사용한 외부 구성 요소

이 프로젝트는 다음 UEFI 헤더 파일을 사용합니다:
- edk2/MdePkg/Include/*.h


## How to USE
```
# 프로젝트 전체 빌드
make

# qemu를 통한 실행 (512MB의 부팅 이미지 제작)
make run
```
