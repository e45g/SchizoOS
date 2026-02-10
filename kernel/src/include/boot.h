#ifndef BOOT_H
#define BOOT_H

#include <common.h>

typedef struct {
    uint64_t framebuffer_base;
    uint64_t framebuffer_size;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
} boot_info_t;

#endif
