#include <cpu/gdt.h>
#include <common.h>

// flag 0-7 access byte
// flag 8-11 flag
uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
    uint64_t descriptor = 0;

    descriptor = limit & 0xF0000;
    descriptor |= (flag << 8) & 0xF0FF00;
    descriptor |= (base >> 16) & 0xFF;
    descriptor |= base & 0xFF000000;

    descriptor <<= 32;

    descriptor |= base << 16;
    descriptor |= limit & 0xFFFF;

    return descriptor;
}

static uint64_t gdt[3] = {0};
static gdt_ptr_t gdt_dscr = {.size = sizeof(gdt)-1, .offset = (uint64_t)&gdt};


void load_gdt()
{
    gdt[0] = create_descriptor(0, 0, 0);
    gdt[1] = create_descriptor(0, 0xFFFFF, 0xA9A);
    gdt[2] = create_descriptor(0, 0xFFFFF, 0xA92);

    asm volatile("lgdt %0" : : "m"(gdt_dscr));

    asm volatile(
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

