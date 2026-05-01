#include <common.h>
#include <cpu/acpi.h>
#include <cpu/apic.h>
#include <debug.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <memory.h>

void acpi_init(void *rsdp_p) {
    rsdp_t *rsdp = (rsdp_t *)rsdp_p;

    if (strncmp((char *)rsdp->signature, "RSD PTR ", 8) != 0) {
        ERROR("ACPI signature does not match\n");
        return;
    }
    OK("ACPI signature correct\n");

    if (rsdp->revision >= 2) {
        xsdt_t *xsdt = (xsdt_t *)rsdp->xsdt_addr;

        if (strncmp((char *)xsdt->header.signature, "XSDT", 4) != 0) {
            ERROR("XSDT signature does not match\n");
            return;
        }
        OK("XSDT signature correct\n");

        uint64_t entry_count =
            (xsdt->header.length - sizeof(acpi_header_t)) / 8;

        for (uint64_t i = 0; i < entry_count; i++) {
            acpi_header_t *entry = (acpi_header_t *)xsdt->entries[i];
            if (strncmp((char *)entry->signature, "APIC", 4) == 0) {
                OK("MADT found\n");
                apic_init((madt_t *)entry);
                break;
            }
        }

    } else {
        ERROR("ACPI 1.0, not supported - Will be added\n");
    }
}
