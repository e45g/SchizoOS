#include <boot.h>
#include <memory.h>
#include <libk/string.h>
#include <kernel.h>

static uint64_t pml4[512] __attribute__((aligned(4096)));

static uint64_t *get_or_create_table(uint64_t *table, uint32_t index) {
    if(!(table[index] & PAGE_PRESENT)) {
        void *new_table = pmm_alloc();
        if(new_table == NULL) PANIC("PMM_ALLOC RETURNED NULL");
        memset(new_table, 0, PAGE_SIZE);
        table[index] = (uintptr_t)new_table | PAGE_PRESENT | PAGE_RW;
    }
    return (uintptr_t*)(table[index] & ~0xFFF);
}

void vmm_init(boot_info_t *boot_info) {
    // kernel
    for (uintptr_t addr = (uintptr_t)&_kernel_start; addr < (uintptr_t)&_kernel_end; addr += PAGE_SIZE)
        vmm_map(addr, addr, PAGE_PRESENT | PAGE_RW);

    // framebuffer
    framebuffer_t fb = boot_info->framebuffer;
    for (uintptr_t addr = (uintptr_t)fb.base_address; addr < (uintptr_t)fb.base_address + fb.buffer_size; addr += PAGE_SIZE)
        vmm_map(addr, addr, PAGE_PRESENT | PAGE_RW);

    // pmm
    pmm_t *pmm = get_pmm();
    uint64_t bitmap_bytes = (pmm->total_frames + 7) / 8;
    for (uintptr_t addr = (uintptr_t)pmm->bitmap; addr < (uintptr_t)pmm->bitmap + bitmap_bytes; addr += PAGE_SIZE)
        vmm_map(addr, addr, PAGE_PRESENT | PAGE_RW);

    // bootinfo
    vmm_map((uintptr_t)boot_info, (uintptr_t)boot_info, PAGE_PRESENT | PAGE_RW);
    vmm_map((uintptr_t)boot_info->rsdp, (uintptr_t)boot_info->rsdp, PAGE_PRESENT | PAGE_RW);

    uint64_t pml4_addr = (uint64_t)pml4;
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4_addr) : "memory");
}

void vmm_map(uintptr_t vaddr, uintptr_t paddr, uint64_t flags) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdp_idx  = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;

    uint64_t *pdpt = get_or_create_table(pml4, pml4_idx);
    uint64_t *pd   = get_or_create_table(pdpt, pdp_idx);
    uint64_t *pt   = get_or_create_table(pd,   pd_idx);

    pt[pt_idx] = (paddr & ~0xFFFULL) | flags;
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

void vmm_unmap(uintptr_t vaddr) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdp_idx  = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & PAGE_PRESENT)) return;
    uint64_t *pdpt = (uint64_t*)(pml4[pml4_idx] & ~0xFFFULL);

    if (!(pdpt[pdp_idx] & PAGE_PRESENT)) return;
    uint64_t *pd = (uint64_t*)(pdpt[pdp_idx] & ~0xFFFULL);

    if (!(pd[pd_idx] & PAGE_PRESENT)) return;
    uint64_t *pt = (uint64_t*)(pd[pd_idx] & ~0xFFFULL);

    if (!(pt[pt_idx] & PAGE_PRESENT)) return;

    pt[pt_idx] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");

    // check if PT is now empty, if so free it
    for (int i = 0; i < 512; i++) {
        if (pt[i] & PAGE_PRESENT) goto pt_not_empty;
    }
    pmm_free((void*)(pd[pd_idx] & ~0xFFFULL));
    pd[pd_idx] = 0;

    pt_not_empty:
    // check if PD is now empty, if so free it
    for (int i = 0; i < 512; i++) {
        if (pd[i] & PAGE_PRESENT) goto pd_not_empty;
    }
    pmm_free((void*)(pdpt[pdp_idx] & ~0xFFFULL));
    pdpt[pdp_idx] = 0;

    pd_not_empty:
    // check if PDP is now empty, if so free it
    for (int i = 0; i < 512; i++) {
        if (pdpt[i] & PAGE_PRESENT) goto pdpt_not_empty;
    }
    pmm_free((void*)(pml4[pml4_idx] & ~0xFFFULL));
    pml4[pml4_idx] = 0;

    pdpt_not_empty:
    return;
}
