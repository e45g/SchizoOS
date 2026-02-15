#include <video.h>
#include <tty.h>

static struct tty tty = {.x=0, .y = 0, .fg = TTY_WHITE, .bg = 0x00000066};

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

    if (tty.y + 16 >= (int)fb->height) {
        tty_clear();
        tty.y = 0;
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
