ARCH ?= x86_64
TARGET = BOOTX64.EFI

# toolchain
CC = gcc
LD ?= ld
OBJCOPY = x86_64-w64-mingw32-objcopy

# gnu-efi paths
EFI_INC = /usr/include/efi
EFI_INC_ARCH = $(EFI_INC)/$(ARCH)
EFI_LIB = /usr/lib
EFI_CRT = $(EFI_LIB)/crt0-efi-$(ARCH).o
EFI_LDS = $(EFI_LIB)/elf_$(ARCH)_efi.lds

# srcs + build
BUILD_DIR ?= ../build/bootloader
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

SO = $(BUILD_DIR)/bootloader.so
EFI = $(BUILD_DIR)/$(TARGET)

# flags
CFLAGS = -I$(EFI_INC) -I$(EFI_INC_ARCH) \
		 -fpic \
		 -ffreestanding \
		 -fno-stack-protector -fno-stack-check -mno-red-zone \
		 -fshort-wchar \
		 -maccumulate-outgoing-args \
		 -Wall \
		 -Wextra \
		 -m64 \
		 -c

LDFLAGS = -shared \
          -Bsymbolic \
		  -nostdlib \
		  -L$(EFI_LIB) -T$(EFI_LDS) \
		  $(EFI_CRT)

LIBS = -lgnuefi -lefi

.PHONY: all clean

all: $(EFI)

build:
	mkdir -p $(BUILD_DIR)

$(EFI): $(SO)
	$(OBJCOPY) \
	    -j .text -j .sdata -j .data -j .rodata \
	    -j .dynamic -j .dynsym \
	    -j .rel -j .rela -j .rel.* -j .rela.* \
	    -j .reloc \
	    --target efi-app-$(ARCH) \
	    --subsystem=10 \
	    $< $@


$(SO): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

$(BUILD_DIR)/%.o: src/%.c | build
	$(CC) $(CFLAGS) -o $@ $<
