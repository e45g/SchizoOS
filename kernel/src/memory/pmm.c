#include <kernel.h>
#include <boot.h>
#include <memory.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <debug.h>

static pmm_t pmm = {0};

static bool is_usable(uint32_t type) {
    switch (type) {
        case UEFI_CONVENTIONAL_MEMORY:
        case UEFI_BOOT_SERVICES_CODE:
        case UEFI_BOOT_SERVICES_DATA:
        case UEFI_LOADER_CODE:
            return true;
        default:
            return false;
    }
}

void pmm_mark_used(uintptr_t addr);
void pmm_init(mmap_t *mmap) {
    uint8_t* map_ptr = (uint8_t*)mmap->map_begin;

    uint64_t kernel_end = (uintptr_t)&_kernel_end;

    uint64_t highest_frame = 0;
    uint64_t total_frames = 0;
    uint64_t free_frames = 0;
    for (uint64_t offset = 0; offset < mmap->map_size; offset += mmap->descriptor_size) {
        uefi_memory_descriptor_t *desc = (uefi_memory_descriptor_t*)(map_ptr + offset);
        total_frames += desc->number_of_pages;

        uint64_t region_end_frame = (desc->physical_start >> PAGE_SHIFT) + desc->number_of_pages;
        if(highest_frame < region_end_frame) highest_frame = region_end_frame;

        if(is_usable(desc->type)) {
            free_frames += desc->number_of_pages;
        }
    }

    if(free_frames == 0 || highest_frame == 0) {
        PANIC("PMM frame error");
    }

    uint64_t bitmap_bytes = (highest_frame + 7) / 8;
    uintptr_t bitmap_location = 0;

    for (uint64_t offset = 0; offset < mmap->map_size; offset += mmap->descriptor_size) {
        uefi_memory_descriptor_t *desc = (uefi_memory_descriptor_t*)(map_ptr + offset);
        uint64_t region_size = desc->number_of_pages << PAGE_SHIFT;

        if(is_usable(desc->type) &&
                    region_size > bitmap_bytes &&
                    desc->physical_start > kernel_end) {
            bitmap_location = desc->physical_start;
            break;
        }
    }

    if(bitmap_location == 0) {
        PANIC("No location found for bitmap");
    }

    pmm.bitmap = (uint8_t *)bitmap_location;
    pmm.total_frames = total_frames;
    memset(pmm.bitmap, 0xFF, bitmap_bytes);

    uint64_t bitmap_start = bitmap_location >> PAGE_SHIFT;
    uint64_t bitmap_end   = (bitmap_location + bitmap_bytes + PAGE_SIZE - 1) >> PAGE_SHIFT;

    for (uint64_t offset = 0; offset < mmap->map_size; offset += mmap->descriptor_size) {
        uefi_memory_descriptor_t *desc = (uefi_memory_descriptor_t*)(map_ptr + offset);

        if(is_usable(desc->type)) {
            uint64_t frame_start = desc->physical_start >> PAGE_SHIFT;
            uint64_t frame_end = frame_start + desc->number_of_pages;
            for(uint64_t frame = frame_start; frame < frame_end; frame++) {
                if(frame >= bitmap_start && frame < bitmap_end) continue;
                pmm.bitmap[frame/8] &= ~(1U << (frame % 8));
                pmm.free_frames++;
            }
        }
    }
    pmm_mark_used(0x0);
}

void pmm_mark_used(uintptr_t addr) {
    uint64_t frame = addr >> PAGE_SHIFT;
    if (pmm.bitmap[frame / 8] & (1U << (frame % 8)))
        return;
    pmm.bitmap[frame / 8] |= (1U << (frame % 8));
    pmm.free_frames--;
}

void pmm_mark_free(uintptr_t addr) {
    uint64_t frame = addr >> PAGE_SHIFT;
    if (!(pmm.bitmap[frame / 8] & (1U << (frame % 8))))
        return;
    pmm.bitmap[frame / 8] &= ~(1U << (frame % 8));
    pmm.free_frames++;
}

void *pmm_alloc() {
    for(uint64_t i = pmm.last_hint; i < pmm.total_frames; i++) {
        if((pmm.bitmap[i/8] & (1U << (i%8))) == 0) {
            pmm.last_hint = i+1;
            pmm_mark_used(i * PAGE_SIZE);
            TRACE("PMM_ALLOC at %p\n", i * PAGE_SIZE);
            return (void*)(i * PAGE_SIZE);
        }
    }

    for(uint64_t i = 0; i < pmm.last_hint; i++) {
        if((pmm.bitmap[i/8] & (1U << (i%8))) == 0) {
            pmm.last_hint = i+1;
            pmm_mark_used(i * PAGE_SIZE);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void *pmm_alloc_zeroed() {
    void *page = pmm_alloc();
    if (page)
        memset(page, 0, PAGE_SIZE);
    return page;
}

void pmm_free(void* addr) {
    pmm_mark_free((uintptr_t)addr);
}

void pmm_info(void) {
    uint64_t total_bytes = pmm.total_frames << PAGE_SHIFT;
    uint64_t free_bytes  = pmm.free_frames  << PAGE_SHIFT;
    uint64_t used_bytes  = total_bytes - free_bytes;

    printf("Memory: %lu MiB total, %lu MiB free, %lu MiB used (%lu KiB)\n",
           total_bytes / (1024 * 1024),
           free_bytes  / (1024 * 1024),
           used_bytes  / (1024 * 1024),
           used_bytes  / 1024);
}

pmm_t *get_pmm() {
    return &pmm;
}
