#ifndef MEMORY_H
#define MEMORY_H

#include <boot.h>
#include <common.h>

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

enum pml4e_flags {
  VMM_PRESENT = 1ULL << 0,
  VMM_RW = 1ULL << 1,
  VMM_US = 1ULL << 2,
  VMM_PWT = 1ULL << 3,
  VMM_PCD = 1ULL << 4,
  VMM_A = 1ULL << 5,
  VMM_XD = 1ULL << 63,
};

typedef enum {
  UEFI_RESERVED = 0,
  UEFI_LOADER_CODE = 1,
  UEFI_LOADER_DATA = 2,
  UEFI_BOOT_SERVICES_CODE = 3,
  UEFI_BOOT_SERVICES_DATA = 4,
  UEFI_RUNTIME_SERVICES_CODE = 5,
  UEFI_RUNTIME_SERVICES_DATA = 6,
  UEFI_CONVENTIONAL_MEMORY = 7,
  UEFI_UNUSABLE_MEMORY = 8,
  UEFI_ACPI_RECLAIM = 9,
  UEFI_ACPI_NVS = 10,
  UEFI_MEMORY_MAPPED_IO = 11,
  UEFI_MEMORY_MAPPED_IO_PORT = 12,
  UEFI_PAL_CODE = 13,
  UEFI_PERSISTENT_MEMORY = 14,
  UEFI_MAX_MEMORY_TYPE = 15
} EFI_MEMORY_TYPE;

typedef struct {
  uint32_t type;
  uint64_t physical_start;
  uint64_t virtual_start;
  uint64_t number_of_pages;
  uint64_t attribute;
} uefi_memory_descriptor_t;

typedef struct {
  uint8_t *bitmap;
  uint64_t total_frames;
  uint64_t free_frames;
  uint64_t last_hint; // last alloc pos
} pmm_t;

pmm_t *get_pmm();
void pmm_init(mmap_t *mmap);
void *pmm_alloc();
void pmm_free(void *addr);
void pmm_info(void);

void vmm_init(boot_info_t *boot_info);
void vmm_map(uintptr_t vaddr, uintptr_t paddr);


#endif
