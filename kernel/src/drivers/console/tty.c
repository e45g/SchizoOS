#include "boot.h"
#include <video.h>
#include <tty.h>

static struct tty tty = {.x=0, .y = 0, .fg = TTY_WHITE, .bg = 0x00000066};

void tty_scroll() {
    framebuffer_t *fb = get_framebuffer();
    uint32_t *pixels = (uint32_t *)fb->base_address;
    uint32_t line_size = fb->pixels_per_scanline;

    // move up
    uint32_t move_count = (fb->height - 16) * line_size;
    for (uint32_t i = 0; i < move_count; i++) {
        pixels[i] = pixels[i + (16 * line_size)];
    }

    // clear the last line
    uint32_t last_line_start = (fb->height - 16) * line_size;
    uint32_t total_pixels = fb->height * line_size;
    for (uint32_t i = last_line_start; i < total_pixels; i++) {
        pixels[i] = tty.bg;
    }
}

void tty_putc(char c) {
    framebuffer_t *fb = get_framebuffer();

    if (c == '\n') {
        tty.x = 0;
        tty.y += 16;
    } else {
        draw_char(c, tty.x, tty.y, tty.fg);
        tty.x += 8;
    }

    if (tty.x >= (int)fb->width) {
        tty.x = 0;
        tty.y += 16;
    }

    if (tty.y + 16 > (int)fb->height) {
        tty_scroll();
        tty.y -= 16;
    }
}

void tty_clear() {
    clean_screen(tty.bg);
}

void tty_set_x(uint16_t x) {
    tty.x = x;
}

void tty_set_y(uint16_t y) {
    tty.y = y;
}

void tty_set_fg(uint32_t color) {
    tty.fg = color;
}

void tty_set_bg(uint32_t color) {
    tty.bg = color;
}
