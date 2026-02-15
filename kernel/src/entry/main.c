#include <common.h>
#include <boot.h>
#include <tty.h>
#include <stdio.h>
#include <cpu/gdt.h>

static boot_info_t *g_boot_info;

void kmain(boot_info_t *boot_info) {
    g_boot_info = boot_info;

    tty_clear();
    printf("Welcome to SchizoOS\n");

    load_gdt();
    printf("[Ok] GDT loaded\n");

    for (;;) __asm__ volatile ("hlt");
}

boot_info_t* get_boot_info() {
    return g_boot_info;
}

framebuffer_t* get_framebuffer() {
    return &g_boot_info->framebuffer;
}
