#ifndef CPU_H
#define CPU_H

#include <common.h>

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdt_ptr_t;

uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag);
void load_gdt();

#endif
