#include <common.h>
#include <boot.h>

#define FONT8x16_IMPLEMENTATION
#include <font8x16.h>

void clean_screen(int32_t color) {
    framebuffer_t *fm = get_framebuffer();
    uint32_t *fb = (uint32_t *)fm->base_address;

    for(uint32_t i = 0; i < fm->pixels_per_scanline * fm->height; i++) {
        fb[i] = color;
    }
}

void draw_char(char c, int x, int y, int32_t color) {
    framebuffer_t *fm = get_framebuffer();
    uint32_t *fb = (uint32_t *)fm->base_address;
    unsigned char *glyph = font8x16[(unsigned char) c];

    for(int row = 0; row < 16; row++) {
        for(int col = 0; col < 8; col++) {
            if(glyph[row] & (0x80 >> col)) {
                fb[(y+row)*fm->pixels_per_scanline + x + col] = color;
            }
        }
    }
}

void draw_string(char *str, int x, int y, int32_t color) {
    while(*str) {
        draw_char(*str++, x, y, color);
        x += 8;
    }
}
