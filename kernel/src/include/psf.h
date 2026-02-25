#ifndef PSF_H
#define PSF_H

#include <common.h>

#define PSF1_MAGIC 0x0436
/*
    For PSF1 glyph width is always = 8 bits
    and glyph height = characterSize
*/
typedef struct {
    uint16_t magic;
    uint8_t fontMode;
    uint8_t characterSize;
} psf1_t;

#define PSF2_MAGIC 0x864ab572
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t num_glyph;
    uint32_t bytes_per_glyph;
    uint32_t height;
    uint32_t width;
} psf2_t;

#endif
