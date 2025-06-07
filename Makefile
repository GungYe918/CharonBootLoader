BUILD_DIR = Build
OUTPUT_DIR = esp/EFI/BOOT

# Arch 설정
ARCH	?=	amd64
include Arch/$(ARCH)/Makefile

MAIN_OBJ = $(BUILD_DIR)/main.obj

OBJS = Graphics/Image/picture.obj Utils/print.obj Graphics/Image/png_loader.obj Graphics/Image/lodepng.obj \
       Utils/mem.obj Utils/guid.obj Utils/stub_asm.obj Load/load.obj Graphics/Screen/init_screen.obj	\
	   Boot/initBoot.obj	Security/hash.obj
OBJ_PATHS = $(addprefix $(BUILD_DIR)/, $(OBJS))

TARGET_FILE = $(OUTPUT_DIR)/$(TARGET)

ifeq ($(ARCH),amd64)
CFLAGS += -DMDE_CPU_X64
OBJS += Utils/stub_asm.obj
endif

ifeq ($(ARCH),aarch64)
CFLAGS += -DMDE_CPU_AARCH64
endif

CFLAGS	+=	-ILib	-ILib/Boot	-ILib/Utils	-ILib/Security

.PHONY: run clean	all

all: $(TARGET_FILE)

# main.c는 고정 처리
$(MAIN_OBJ): main.c
	mkdir -p $(BUILD_DIR)
	clang -target $(ARCH)-windows -c $(CFLAGS) $< -o $@

# .obj 파일 빌드 규칙 - 규칙 형식 수정
$(BUILD_DIR)/%.obj:
	@src_c=`find Lib -wholename "*/$*.c" | head -n1`; \
	src_s=`find Lib -wholename "*/$*.S" -o -wholename "*/$*.s" | head -n1`; \
	if [ -n "$$src_c" ]; then \
		echo "Compiling C $$src_c -> $@"; \
		mkdir -p $(dir $@); \
		clang -target $(ARCH)-windows -c $(CFLAGS) $$src_c -o $@; \
	elif [ -n "$$src_s" ]; then \
		echo "Assembling ASM $$src_s -> $@"; \
		mkdir -p $(dir $@); \
		clang -target $(ARCH)-windows -c $$src_s -o $@; \
	else \
		echo "Error: source file for $@ not found"; exit 1; \
	fi

# 링크
$(TARGET_FILE): $(MAIN_OBJ) $(OBJ_PATHS)
	mkdir -p $(OUTPUT_DIR)
	lld-link $(LDFLAGS) $(MAIN_OBJ) $(OBJ_PATHS) /out:$(TARGET_FILE)
	mv -f $(OUTPUT_DIR)/$(basename $(TARGET)).lib $(BUILD_DIR) 2>/dev/null || true


clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(TARGET_FILE)


run:
ifeq ($(ARCH),amd64)
	./run/run_amd64.sh
else ifeq ($(ARCH),aarch64)
	./run/run_aarch64.sh
else
	@echo "Unsupported ARCH: $(ARCH)"
endif