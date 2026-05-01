#ifndef APIC_H
#define APIC_H

#include "cpu/acpi.h"

#define APIC_SVR 0xF0
#define APIC_EOI 0xB0

typedef struct {
    acpi_header_t header;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t  entries[];
} madt_t;


void apic_init(madt_t *madt);
void lapic_write(uint32_t reg, uint32_t val);

#endif
