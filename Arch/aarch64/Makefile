ARCH            = aarch64
TARGET          = BOOTAA64.EFI
TARGET_TRIPLE   = aarch64-windows
MACHINE_FLAG    = /machine:arm64

CFLAGS += -ffreestanding -fshort-wchar -Wall \
          -IInclude -IInclude/Protocol -IInclude/Arm \
          -fno-stack-check -fno-stack-protector     \
          -fno-pic -fno-pie

LDFLAGS += /subsystem:efi_application /entry:UefiMain $(MACHINE_FLAG) \
           /nodefaultlib /opt:ref /dll /align:4096  /export:UefiMain

