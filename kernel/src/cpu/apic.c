#include <debug.h>
#include <memory.h>
#include <cpu/apic.h>
#include <cpu/io.h>

uint64_t lapic_addr = 0;
uint64_t ioapic_addr = 0;
uint8_t  gsi_override[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

uint32_t lapic_read(uint32_t reg) {
    return *(volatile uint32_t*)(lapic_addr + reg);
}

void lapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)(lapic_addr + reg) = val;
}

void pic_disable(void) {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void parse_madt(madt_t *madt) {
    lapic_addr = madt->lapic_addr;

    uint8_t *entry = madt->entries;
    uint8_t *end = (uint8_t*)madt + madt->header.length;

    while(entry < end) {
        uint8_t type = entry[0];
        uint8_t length = entry[1];

        if(type == 1) { // IO APIC
            ioapic_addr = *(uint32_t *)(entry + 4);
        }

        if(type == 2) {
            uint8_t irq = entry[3];
            uint32_t gsi = *(uint32_t *)(entry + 4);
            gsi_override[irq] = gsi;
        }
        if(type == 5) { // lapic overrride
            lapic_addr = *(uint64_t *)(entry + 4);
        }
        entry += length;
    }
}


uint32_t ioapic_read(uint8_t reg) {
    *(volatile uint32_t*)(ioapic_addr + 0x00) = reg;
    return *(volatile uint32_t*)(ioapic_addr + 0x10);
}

void ioapic_write(uint8_t reg, uint32_t val) {
    *(volatile uint32_t*)(ioapic_addr + 0x00) = reg;
    *(volatile uint32_t*)(ioapic_addr + 0x10) = val;
}

void ioapic_route(uint8_t irq, uint8_t vector) {
    uint8_t gsi = gsi_override[irq];
    uint64_t entry = vector;

    uint8_t reg = 0x10+gsi*2;

    ioapic_write(reg, (uint32_t) entry);
    ioapic_write(reg+1, (uint32_t)(entry >> 32));
}

void apic_init(madt_t *madt) {
    parse_madt(madt);

    pic_disable();
    lapic_write(APIC_SVR, lapic_read(APIC_SVR) | 0x1FF);

    ioapic_route(1, 0x21);

    // flush any pending keyboard input
    while(inb(0x64) & 0x01) {
        inb(0x60);
    }
}
