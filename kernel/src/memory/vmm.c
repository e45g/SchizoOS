#include <boot.h>
#include <memory.h>
#include <libk/string.h>
#include <kernel.h>

static uint64_t pml4[512] __attribute__((aligned(4096)));

static uint64_t *get_or_create_table(uint64_t *table, uint32_t index) {
    if(!(table[index] & VMM_PRESENT)) {
        void *new_table = pmm_alloc();
        if(new_table == NULL) PANIC("PMM_ALLOC RETURNED NULL");
        memset(new_table, 0, PAGE_SIZE);
        table[index] = (uintptr_t)new_table | VMM_PRESENT | VMM_RW;
    }
    return (uintptr_t*)(table[index] & ~0xFFF);
}

void vmm_init(boot_info_t *boot_info) {
    uintptr_t k_start = (uintptr_t)&_kernel_start;
    uintptr_t k_end = (uintptr_t)&_kernel_end;
    for (uintptr_t addr = k_start; addr < k_end; addr += PAGE_SIZE) {
        vmm_map(addr, addr);
    }

    framebuffer_t fb = boot_info->framebuffer;
    uintptr_t fb_base = (uintptr_t) fb.base_address;
    for(uintptr_t addr = fb_base; addr < (fb_base + fb.buffer_size); addr += PAGE_SIZE) {
        vmm_map(addr, addr);
    }

    pmm_t *pmm = get_pmm();
    uint8_t *bitmap_loc = pmm->bitmap;
    for(uintptr_t addr = (uintptr_t) bitmap_loc; addr < (uintptr_t) bitmap_loc + pmm->total_frames; addr += PAGE_SIZE) {
        vmm_map(addr, addr);
    }

    uint64_t pml4_addr = (uint64_t)pml4;
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4_addr) : "memory");
}

void vmm_map(uintptr_t vaddr, uintptr_t paddr) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdp_idx  = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;

    uint64_t *pdpt = get_or_create_table(pml4, pml4_idx);
    uint64_t *pd   = get_or_create_table(pdpt, pdp_idx);
    uint64_t *pt   = get_or_create_table(pd,   pd_idx);

    pt[pt_idx] = (paddr & ~0xFFFULL) | VMM_PRESENT | VMM_RW;
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

void vmm_unmap(uintptr_t vaddr) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdp_idx  = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & VMM_PRESENT)) return;
    uint64_t *pdpt = (uint64_t*)(pml4[pml4_idx] & ~0xFFFULL);

    if (!(pdpt[pdp_idx] & VMM_PRESENT)) return;
    uint64_t *pd = (uint64_t*)(pdpt[pdp_idx] & ~0xFFFULL);

    if (!(pd[pd_idx] & VMM_PRESENT)) return;
    uint64_t *pt = (uint64_t*)(pd[pd_idx] & ~0xFFFULL);

    if (!(pt[pt_idx] & VMM_PRESENT)) return;

    pt[pt_idx] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");

    // check if PT is now empty, if so free it
    for (int i = 0; i < 512; i++) {
        if (pt[i] & VMM_PRESENT) goto pt_not_empty;
    }
    pmm_free((void*)(pd[pd_idx] & ~0xFFFULL));
    pd[pd_idx] = 0;

    pt_not_empty:
    // check if PD is now empty, if so free it
    for (int i = 0; i < 512; i++) {
        if (pd[i] & VMM_PRESENT) goto pd_not_empty;
    }
    pmm_free((void*)(pdpt[pdp_idx] & ~0xFFFULL));
    pdpt[pdp_idx] = 0;

    pd_not_empty:
    // check if PDP is now empty, if so free it
    for (int i = 0; i < 512; i++) {
        if (pdpt[i] & VMM_PRESENT) goto pdpt_not_empty;
    }
    pmm_free((void*)(pml4[pml4_idx] & ~0xFFFULL));
    pml4[pml4_idx] = 0;

    pdpt_not_empty:
    return;
}
