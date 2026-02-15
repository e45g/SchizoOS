#include <cpu/gdt.h>
#include <libk/stdio.h>
#include <common.h>

void create_gdt_entry(gdt_entry_t *entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_mid = (base >> 16) & 0xFF;
    entry->access = access;
    entry->limit_high_flags = ((flags & 0xF) << 4) | ((limit >> 16) & 0xF);
    entry->base_high = (base>>24) & 0xFF;
}

static gdt_entry_t gdt[3] = {0};
static gdtr_t gdtr;


void load_gdt()
{
    create_gdt_entry(&gdt[0], 0, 0, 0, 0);
    create_gdt_entry(&gdt[1], 0, 0xFFFFF, KERNEL_CODE_SEG, FLAG);
    create_gdt_entry(&gdt[2], 0, 0xFFFFF, KERNEL_DATA_SEG, FLAG);

    gdtr.size = sizeof(gdt) - 1;
    gdtr.offset = (uint64_t)&gdt;

    __asm__ volatile("lgdt %0" : : "m"(gdtr));

    __asm__ volatile(
        "pushq $0x08\n\t"
        "leaq 1f(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%ss\n\t"
        :
        :
        : "rax", "memory"
    );
}

