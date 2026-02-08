ARCH ?= x86_64
TARGET = BOOTX64.EFI

# toolchain
CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy

# gnu-efi paths
EFI_INC = /usr/include/efi
EFI_INC_ARCH = $(EFI_INC)/$(ARCH)
EFI_LIB = /usr/lib
EFI_CRT = $(EFI_LIB)/crt0-efi-$(ARCH).o
EFI_LDS = $(EFI_LIB)/elf_$(ARCH)_efi.lds

# srcs + build
BUILD_DIR ?= ../build
BOOT_DIR = $(BUILD_DIR)/bootloader
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,$(BOOT_DIR)/%.o,$(SRCS))

SO = $(BOOT_DIR)/bootloader.so
EFI = $(BOOT_DIR)/$(TARGET)

# flags
CFLAGS ?= -I$(EFI_INC) -I$(EFI_INC_ARCH) \
		 -fpic \
		 -ffreestanding \
		 -fno-stack-protector -fno-stack-check -mno-red-zone \
		 -fshort-wchar \
		 -maccumulate-outgoing-args \
		 -Wall \
		 -Wextra \
		 -m64 \
		 -c

LDFLAGS ?= -shared \
          -Bsymbolic \
		  -nostdlib \
		  -L$(EFI_LIB) -T$(EFI_LDS) \
		  $(EFI_CRT)

LIBS = -lgnuefi -lefi

.PHONY: all clean

all: $(EFI)

$(BOOT_DIR):
	mkdir -p $(BOOT_DIR)

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

$(BOOT_DIR)/%.o: src/%.c | $(BOOT_DIR)
	$(CC) $(CFLAGS) -o $@ $<
