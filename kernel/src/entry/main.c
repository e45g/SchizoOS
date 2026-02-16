#include <cpu/idt.h>
#include <common.h>
#include <boot.h>
#include <tty.h>
#include <libk/stdio.h>
#include <memory.h>
#include <cpu/gdt.h>

static boot_info_t *g_boot_info;

void kmain(boot_info_t *boot_info) {
    g_boot_info = boot_info;

    pmm_init();

    tty_clear();

    uintptr_t gdtr_addr = gdt_init();
    printf("[Ok] GDT loaded at %p\n", gdtr_addr);

    uintptr_t idtr_addr = idt_init();
    printf("[Ok] IDT loaded at %p\n", idtr_addr);

    printf("\nWelcome to SchizoOS\n");

    pmm_info();

    for (;;) __asm__ volatile ("hlt");
}

boot_info_t* get_boot_info() {
    return g_boot_info;
}

framebuffer_t* get_framebuffer() {
    return &g_boot_info->framebuffer;
}

memory_map_t* get_memmap() {
    return &g_boot_info->memory_map;
}
