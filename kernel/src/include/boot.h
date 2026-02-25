#ifndef BOOT_H
#define BOOT_H

#include <common.h>

typedef struct {
    void* base_address;
    uint64_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
} framebuffer_t;

typedef struct {
    void* map_begin;
    uint64_t map_size;
    uint64_t descriptor_size;
} mmap_t;

typedef struct {
    framebuffer_t framebuffer;
    mmap_t  mmap;
    void* rsdp;
} boot_info_t;

#endif
