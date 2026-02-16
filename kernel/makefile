ARCH ?= x86_64
TARGET = kernel.elf

# toolchain
CC = gcc
LD ?= ld
NASM ?= nasm
OBJCOPY ?= objcopy

# srcs + build
BUILD_DIR ?= ../build/kernel
C_SRCS = $(shell find src -name '*.c')
ASM_SRCS = $(shell find src -name '*.asm')
C_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
ASM_OBJS = $(patsubst src/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))
OBJS = $(C_OBJS) $(ASM_OBJS)
KERNEL_LD = linker.ld

ELF = $(BUILD_DIR)/$(TARGET)

# flags
CFLAGS = -I./src/include -ffreestanding \
                -fno-stack-protector \
                -fno-stack-check \
                -mno-red-zone \
                -Wall \
                -Wextra \
                -m64 \
                -mno-sse \
                -mno-mmx \
                -mgeneral-regs-only \
                -c

NASMFLAGS = -f elf64 \
            -i ./src/include/

LDFLAGS = -nostdlib \
          -T$(KERNEL_LD)

.PHONY: all clean

all: $(ELF)

$(ELF): $(OBJS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: src/%.asm | $(BUILD_DIR)
	@mkdir -p $(@D)
	$(NASM) $(NASMFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
