#ifndef TTY_H
#define TTY_H

#include <common.h>

#define TTY_RED 0x00FF0000
#define TTY_GREEN 0x0000FF00
#define TTY_BLUE 0x000000FF
#define TTY_WHITE 0x00FFFFFF

struct tty {
    uint16_t x;
    uint16_t y;
    uint32_t fg;
    uint32_t bg;
};

void tty_set_fg(uint32_t color);
void tty_set_bg(uint32_t color);
void tty_set_x(uint16_t x);
void tty_set_y(uint16_t y);
void tty_putc(char c);
void tty_clear();

#endif
