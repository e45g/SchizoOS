#include <common.h>
#include <memory.h>
#include <boot.h>
#include <libk/string.h>

#include <psf.h>

extern char _binary_assets_fonts_zap_vga16_psf_start;
static uint32_t* fb_base;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;

static uint32_t font_width;
static uint32_t font_height;
static uint32_t font_bytes_per_glyph;
static uint32_t font_header_size;
static void* font_ptr = NULL;

void fb_init(framebuffer_t *fb) {
    fb_base = (uint32_t*)fb->base_address;
    fb_width = fb->width;
    fb_height = fb->height;
    fb_pitch = fb->pixels_per_scanline;

    void* raw_font = &_binary_assets_fonts_zap_vga16_psf_start;
    psf2_t *font2 = (psf2_t *)raw_font;
    psf1_t *font1 = (psf1_t *)raw_font;

    if (font2->magic == PSF2_MAGIC) {
        font_ptr = raw_font;
        font_width = font2->width;
        font_height = font2->height;
        font_bytes_per_glyph = font2->bytes_per_glyph;
        font_header_size = font2->header_size;
    }
    else if (font1->magic == PSF1_MAGIC) {
        font_ptr = raw_font;
        font_width = 8; // PSF1 is always 8 pixels wide
        font_height = font1->characterSize;
        font_bytes_per_glyph = font1->characterSize;
        font_header_size = 4; // PSF1 header is always 4 bytes
    }

    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        fb_base[i] = 0x00000000;
    }
}

void fb_putpixel(int x, int y, int32_t color) {
    fb_base[y * fb_pitch + x] = color;
}


void fb_draw_char(char c, int x, int y, int32_t fg, int32_t bg) {
    if (!font_ptr) return;

    unsigned char *glyph = (unsigned char *)font_ptr + font_header_size +
                           (c * font_bytes_per_glyph);

    uint32_t bytes_per_line = (font_width + 7) / 8;

    for (uint32_t cy = 0; cy < font_height; cy++) {
        for (uint32_t cx = 0; cx < font_width; cx++) {
            unsigned char bitmask = 0x80 >> (cx % 8);
            if (glyph[cy * bytes_per_line + (cx / 8)] & bitmask) {
                fb_putpixel(x + cx, y + cy, fg);
            }
            else{
                fb_putpixel(x+cx, y+cy, bg);
            }
        }
    }
}

void fb_draw_string(char *str, int x, int y, int32_t fg, int32_t bg) {
    while(*str) {
        fb_draw_char(*str++, x, y, fg, bg);
        x += font_width;
    }
}

void fb_clean_screen(int32_t color) {
    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        fb_base[i] = color;
    }
}

void fb_scroll(uint32_t bg_color) {
    uint32_t scroll_rows = font_height;
    uint32_t line_size = fb_pitch;

    uint32_t move_count = (fb_height - scroll_rows) * line_size;

    for (uint32_t i = 0; i < move_count; i++) {
        fb_base[i] = fb_base[i + (scroll_rows * line_size)];
    }

    uint32_t last_line_start = (fb_height - scroll_rows) * line_size;
    uint32_t total_pixels = fb_height * line_size;
    for (uint32_t i = last_line_start; i < total_pixels; i++) {
        fb_base[i] = bg_color;
    }
}

uint32_t fb_get_width()  { return fb_width;  }
uint32_t fb_get_height() { return fb_height; }

void fb_get_font_dims(uint32_t* width, uint32_t* height) {
    if (font_ptr) {
        *width  = font_width;
        *height = font_height;
    } else {
        *width = 8; *height = 16; // Fallback
    }
}
