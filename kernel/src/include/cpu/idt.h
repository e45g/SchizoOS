#ifndef IDT_H
#define IDT_H

#include <common.h>

#define IDT_MAX_DESCRIPTORS 256

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) idtr_t;

typedef struct {
    uint16_t offset_low;
    uint16_t kernel_cs;
    uint8_t  ist; // IST in the TSS, set to zero for now
    uint8_t  attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
	uint32_t reserved; // set to zero
} __attribute__((packed)) idt_entry_t;

uintptr_t idt_init();

#endif
