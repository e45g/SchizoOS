#include <kernel.h>
#include <boot.h>
#include <memory.h>
#include <libk/string.h>

// todo
// total_frames using conventional memory only? ill have to think about it more
// EfiLoaderData/Code should be also free? KERNEL IS EFILOADERDATA!!

static pmm_t pmm = {0};

void pmm_init() {
    memory_map_t *mmap = get_memmap();

    uint64_t kernel_end = (uintptr_t)&_kernel_end;

    uint8_t* map_ptr = (uint8_t*)mmap->map_begin;

    uint64_t total_frames = 0;
    for (uint64_t offset = 0; offset < mmap->map_size; offset += mmap->descriptor_size) {
        uefi_memory_descriptor_t *desc = (uefi_memory_descriptor_t*)(map_ptr + offset);

        if(desc->type == UEFI_CONVENTIONAL_MEMORY) {
            total_frames += desc->number_of_pages;
        }
    }

    if(total_frames == 0) {
        PANIC("No memory?");
    }

    uint64_t bitmap_bytes = (total_frames + 7) / 8;
    uintptr_t bitmap_location = 0;

    for (uint64_t offset = 0; offset < mmap->map_size; offset += mmap->descriptor_size) {
        uefi_memory_descriptor_t *desc = (uefi_memory_descriptor_t*)(map_ptr + offset);
        uint64_t region_size = desc->number_of_pages << PAGE_SHIFT;

        if(desc->type == UEFI_CONVENTIONAL_MEMORY &&
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

        if(desc->type == UEFI_CONVENTIONAL_MEMORY) {
            uint64_t frame_start = desc->physical_start >> PAGE_SHIFT;
            uint64_t frame_end = frame_start + desc->number_of_pages;
            for(uint64_t frame = frame_start; frame < frame_end; frame ++) {
                if(frame >= bitmap_start && frame < bitmap_end) continue;
                pmm.bitmap[frame/8] &= ~(1 << (frame % 8));
                pmm.free_frames++;
            }
        }
    }
}

void pmm_mark_used(uintptr_t addr) {
    uint32_t page = addr / PAGE_SIZE;
    pmm.bitmap[page / 8] |= (1 << (page % 8));
    pmm.free_frames--;
}

void pmm_mark_free(uintptr_t addr) {
    uint32_t page = addr / PAGE_SIZE;
    pmm.bitmap[page / 8] &= ~(1 << (page % 8));
    pmm.free_frames++;
}

void *pmm_alloc() {
for(uint64_t i = pmm.last_hint; i < pmm.total_frames; i++) {
        if((pmm.bitmap[i/8] & (1 << (i%8))) == 0) {
            pmm.last_hint = i;
            pmm_mark_used(i * PAGE_SIZE);
            return (void*)(i * PAGE_SIZE);
        }
    }

    for(uint64_t i = 0; i < pmm.last_hint; i++) {
        if((pmm.bitmap[i/8] & (1 << (i%8))) == 0) {
            pmm.last_hint = i;
            pmm_mark_used(i * PAGE_SIZE);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_page(void* addr) {
    pmm_mark_free((uintptr_t)addr);
}

void pmm_info(void) {
    uint64_t free = pmm.free_frames << PAGE_SHIFT;
    uint64_t total = pmm.total_frames << PAGE_SHIFT;
    uint64_t used = total - free;

    printf("Memory: %lu MB total, %lu MB free, %lu KiB used\n",
           total / (1024 * 1024),
           free / (1024 * 1024),
           used / (1024));
}
