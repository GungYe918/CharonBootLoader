ARCH        = x86_64
TARGET      = BOOTX64.EFI
TARGET_TRIPLE = x86_64-windows
MACHINE_FLAG  = /machine:x64

CFLAGS += -ffreestanding -fshort-wchar -Wall -IInclude -IInclude/Protocol \
          -fno-stack-check -fno-stack-protector	\
		  -IInclude -IInclude/X64 -IInclude/Protocol

LDFLAGS += /subsystem:efi_application /entry:UefiMain $(MACHINE_FLAG) \
           /nodefaultlib /opt:ref /dll



