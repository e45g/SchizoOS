ARCH = x86_64

# toolchain
CC = gcc
LD = ld
OBJCOPY = objcopy

# paths
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso
EFI_IMG = $(ISO_DIR)/efi.img
BOOTLOADER_DIR = bootloader
KERNEL_DIR = kernel
KERNEL_ELF = $(BUILD_DIR)/kernel/kernel.elf
EFI = $(BUILD_DIR)/bootloader/BOOTX64.EFI
ISO = SchizoOS.iso

OVMF = /usr/share/edk2/x64/OVMF.4m.fd

.PHONY: all clean iso bootloader kernel run debug

all: clean bootloader kernel iso run

clean:
	rm -rf $(BUILD_DIR) $(ISO)

bootloader:
	$(MAKE) -C $(BOOTLOADER_DIR)

kernel:
	$(MAKE) -C $(KERNEL_DIR)

iso: $(EFI) $(KERNEL_ELF)
	rm -rf $(ISO_DIR) && mkdir -p $(ISO_DIR)
	dd if=/dev/zero of=$(EFI_IMG) bs=1M count=4
	mformat -i $(EFI_IMG) ::
	mmd -i $(EFI_IMG) ::/EFI ::/EFI/BOOT
	mcopy -i $(EFI_IMG) $(EFI) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $(EFI_IMG) $(KERNEL_ELF) ::/kernel.elf
	xorriso -as mkisofs -o $(ISO) --efi-boot efi.img \
	    -efi-boot-part --efi-boot-image $(ISO_DIR)

run: iso
	qemu-system-x86_64 \
	    -bios $(OVMF) \
	    -cdrom $(ISO) \
	    -net none

debug: iso
	qemu-system-x86_64 \
	    -bios $(OVMF) \
	    -cdrom $(ISO) \
	    -net none \
	    -serial stdio \
		-d int -no-shutdown -no-reboot
