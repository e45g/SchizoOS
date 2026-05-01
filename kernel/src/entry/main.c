#include <debug.h>
#include <cpu/idt.h>
#include <common.h>
#include <boot.h>
#include <drivers/tty.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <memory.h>
#include <cpu/gdt.h>
#include <drivers/keyboard.h>
#include <video.h>
#include <cpu/acpi.h>

void kmain(boot_info_t *boot_info) {
    fb_init(&boot_info->framebuffer);
    tty_init();
    OK("Framebuffer at %p\n", boot_info->framebuffer.base_address);

    pmm_init(&boot_info->mmap);
    OK("PMM init\n");

    uintptr_t gdtr_addr = gdt_init();
    OK("GDT loaded at %p\n", gdtr_addr);

    uintptr_t idtr_addr = idt_init();
    OK("IDT loaded at %p\n", idtr_addr);

    // vmm_init(boot_info);
    // OK("VMM init\n");

    acpi_init(boot_info->rsdp);
    __asm__ volatile("sti");

    printf("Welcome to SchizoOS\n");
    pmm_info();

    while(1) {
        char c = getchar();
        printf("%c", c);
    }

    for (;;) __asm__ volatile ("hlt");
}
