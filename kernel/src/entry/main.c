#include "debug.h"
#include <cpu/idt.h>
#include <common.h>
#include <boot.h>
#include <tty.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <memory.h>
#include <cpu/gdt.h>
#include <video.h>

void kmain(boot_info_t *boot_info) {
    fb_init(&boot_info->framebuffer);
    tty_init();
    OK("[Ok] Framebuffer at %p\n", boot_info->framebuffer.base_address);

    pmm_init(&boot_info->mmap);
    OK("[Ok] PMM init\n");

    uintptr_t gdtr_addr = gdt_init();
    OK("[Ok] GDT loaded at %p\n", gdtr_addr);

    uintptr_t idtr_addr = idt_init();
    OK("[Ok] IDT loaded at %p\n", idtr_addr);

    vmm_init(boot_info);
    OK("VMM init\n");

    printf("Welcome to SchizoOS\n");

    pmm_info();


    for (;;) __asm__ volatile ("hlt");
}
