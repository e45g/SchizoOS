#ifndef GDT_H
#define GDT_H

#include <common.h>

#define SEG_PRESENT(x)   ((x) << 0x7) // segment present
#define SEG_PRIV(x)      (((x) &  0x03) << 0x05) // set ring, 0 = kernel, 3 = user
#define SEG_DESCTYPE(x)  ((x) << 4) // 0 for system, 1 for code/data
#define SEG_EXEC(x)      ((x) << 3) // 1 for code, 0 for data
#define SEG_DC(x)        ((x) << 2) // direction/access level bit
#define SEG_RW(x)        ((x) << 1) // read/write bit
#define SEG_AC(x)        ((x) << 0) // access bit, leave at 1 unless needed otherwise

#define SEG_DATA_RD      0x01 // Read-Only, accessed
#define SEG_DATA_RDWR    0x03 // Read/Write, accessed
#define SEG_CODE_EX      0x01 // Execute-Only, accessed
#define SEG_CODE_EXRD    0x03 // Execute/Read, accessed
#define SEG_CODE_EXC     0x05 // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC   0x07 // Execute/Read, conforming, accessed

#define FLAG_GRAN(x)     ((x) << 3) // 0 for 1 byte limit blocks, 1 for 4 KiB ones
#define FLAG_DB(x)       ((x) << 2) // size flag
#define FLAG_LONG(x)     ((x) << 1) // long mode, if 1, db should be 0

#define KERNEL_CODE_SEG SEG_PRESENT(1) | SEG_PRIV(0)    | SEG_DESCTYPE(1) | \
                        SEG_EXEC(1)    | SEG_CODE_EXRD

#define KERNEL_DATA_SEG SEG_PRESENT(1) | SEG_PRIV(0)    | SEG_DESCTYPE(1) | \
                        SEG_EXEC(0)    | SEG_DATA_RDWR

#define FLAG FLAG_GRAN(1) | FLAG_DB(0) | FLAG_LONG(1)

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high_flags; // low 4 bits = flags
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdtr_t;

uintptr_t gdt_init();

#endif
