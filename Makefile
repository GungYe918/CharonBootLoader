ARCH         = x86_64
TARGET       = BOOTX64.EFI
BUILD_DIR    = Build
OUTPUT_DIR   = esp/EFI/BOOT
CFLAGS       = -ffreestanding -fshort-wchar -Wall -IInclude -IInclude/X64 -IInclude/Protocol	\
				-fno-stack-check -fno-stack-protector
LDFLAGS      = /subsystem:efi_application /entry:UefiMain /machine:x64 /nodefaultlib /opt:ref /dll

MAIN_OBJ     = $(BUILD_DIR)/main.obj


OBJS         = Graphics/Image/picture.obj	Utils/print.obj	Graphics/Image/png_loader.obj	Graphics/Image/lodepng.obj	Utils/mem.obj	\
				Utils/guid.obj	Utils/stub.obj	Utils/stub_asm.obj	Elf/loadElf.obj	Graphics/Screen/init_screen.obj

# 내부 처리용 (Build/*.obj로 변환)
OBJ_PATHS    = $(addprefix $(BUILD_DIR)/, $(OBJS))
TARGET_FILE  = $(OUTPUT_DIR)/$(TARGET)

all: $(TARGET_FILE)

# main.c는 고정 처리
$(MAIN_OBJ): main.c
	mkdir -p $(BUILD_DIR)
	clang -target x86_64-windows -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/%.obj:
	@src_c=`find Lib -wholename "*/$*.c" | head -n1`; \
	src_s=`find Lib -wholename "*/$*.S" -o -wholename "*/$*.s" | head -n1`; \
	if [ -n "$$src_c" ]; then \
		echo "Compiling C $$src_c -> $@"; \
		mkdir -p $(dir $@); \
		clang -target x86_64-windows -c $(CFLAGS) $$src_c -o $@; \
	elif [ -n "$$src_s" ]; then \
		echo "Assembling ASM $$src_s -> $@"; \
		mkdir -p $(dir $@); \
		clang -target x86_64-windows -c $$src_s -o $@; \
	else \
		echo "Error: source file for $@ not found"; exit 1; \
	fi

# 링크
$(TARGET_FILE): $(MAIN_OBJ) $(OBJ_PATHS)
	mkdir -p $(OUTPUT_DIR)
	lld-link $(LDFLAGS) $(MAIN_OBJ) $(OBJ_PATHS) /out:$(TARGET_FILE)
	mv -f $(OUTPUT_DIR)/BOOTX64.lib $(BUILD_DIR) 2>/dev/null || true

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET_FILE)

run:
	./run.sh
