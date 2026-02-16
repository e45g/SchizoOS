#include <tty.h>
#include <cpu/idt.h>
#include <libk/stdio.h>

static bool vectors[IDT_MAX_DESCRIPTORS];
static idtr_t idtr;
__attribute__((aligned(0x10))) static idt_entry_t idt[IDT_MAX_DESCRIPTORS];
extern void* isr_stub_table[];

__attribute__((noreturn))
void exception_handler(uint32_t vector, uint32_t error_code) {
    tty_set_fg(TTY_WHITE);
    tty_set_bg(0x00660000);

    tty_clear();
    tty_set_x(0);
    tty_set_y(0);

    printf("PANIC PANIC!!!\nWhat did you do!?\n\nException: %d\nError Code: %d\n", vector, error_code);

    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

void idt_set_descriptor(uint8_t vector, void* offset, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->offset_low     = (uint64_t)offset & 0xFFFF;
    descriptor->kernel_cs      = 0x08;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->offset_mid     = ((uint64_t)offset >> 16) & 0xFFFF;
    descriptor->offset_high    = ((uint64_t)offset >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}


uintptr_t idt_init() {
    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uintptr_t)&idt;

    uint8_t gate_types[32] = {
        0x8E, // 0: Divide Error (Fault)
        0x8F, // 1: Debug Exception (Trap)
        0x8E, // 2: NMI Interrupt (Interrupt)
        0x8F, // 3: Breakpoint (Trap)
        0x8F, // 4: Overflow (Trap)
        0x8E, // 5: BOUND Range Exceeded (Fault)
        0x8E, // 6: Invalid Opcode (Fault)
        0x8E, // 7: Device Not Available (Fault)
        0x8E, // 8: Double Fault (Abort)
        0x8E, // 9: Coprocessor Segment Overrun (Fault) [obsolete]
        0x8E, // 10: Invalid TSS (Fault)
        0x8E, // 11: Segment Not Present (Fault)
        0x8E, // 12: Stack-Segment Fault (Fault)
        0x8E, // 13: General Protection (Fault)
        0x8E, // 14: Page Fault (Fault)
        0x8E, // 15: Reserved
        0x8E, // 16: x87 FPU Floating-Point Error (Fault)
        0x8F, // 17: Alignment Check (Trap)
        0x8E, // 18: Machine Check (Abort)
        0x8E, // 19: SIMD Floating-Point Exception (Fault)
        0x8E, // 20: Virtualization Exception (Fault)
        0x8E, // 21: Control Flow Exception (Fault)
        0x8E, // 22: Reserved
        0x8E, // 23: Reserved
        0x8E, // 24: Reserved
        0x8E, // 25: Reserved
        0x8E, // 26: Reserved
        0x8E, // 27: Reserved
        0x8E, // 28: Hypervisor Injection Exception (Fault)
        0x8E, // 29: VMM Communication Exception (Fault)
        0x8E, // 30: Security Exception (Fault)
        0x8E, // 31: Reserved
    };

    for(uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], gate_types[vector]);
        vectors[vector] = true;
    }

    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile ("sti");

    return (uintptr_t)&idtr;
}
