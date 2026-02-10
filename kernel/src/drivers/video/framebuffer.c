#include <common.h>
#include <boot.h>

#define FONT8x16_IMPLEMENTATION
#include <font8x16.h>

void draw_char(boot_info_t *bt, char c, int x, int y, int32_t color) {
    uint32_t *fb = (uint32_t *)bt->framebuffer_base;
    unsigned char *glyph = font8x16[(unsigned char) c];

    for(int row = 0; row < 16; row++) {
        for(int col = 0; col < 8; col++) {
            if(glyph[row] & (0x80 >> col)) {
                fb[(y+row)*bt->pixels_per_scanline + x + col] = color;
            }
        }
    }
}

void draw_string(boot_info_t *bt, char *str, int x, int y, int32_t color) {
    while(*str) {
        draw_char(bt, *str++, x, y, color);
        x += 8;
    }
}
