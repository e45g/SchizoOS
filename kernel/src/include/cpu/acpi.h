#ifndef ACPI_H
#define ACPI_H

#include <common.h>

typedef struct {
    uint8_t   signature[8];
    uint8_t   checksum;
    uint8_t   oemid[6];
    uint8_t   revision;
    uint32_t  rsdt_addr;
    uint32_t  length;
    uint64_t xsdt_addr;
    uint8_t   ext_checksum;
    uint8_t   reserved[3];
} __attribute__((packed)) rsdp_t;

typedef struct {
    uint8_t   signature[4];
    uint32_t  length;
    uint8_t   revision;
    uint8_t   checksum;
    uint8_t   oemid[6];
    uint8_t   oem_table_id[8];
    uint32_t  oem_revision;
    uint32_t  creator_id;
    uint32_t  creator_revision;
} __attribute__((packed)) acpi_header_t;

typedef struct {
    acpi_header_t header;
    uint64_t entries[];
} __attribute__((packed)) xsdt_t;

void acpi_init(void *rsdp);

#endif
