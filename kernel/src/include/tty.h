#ifndef TTY_H
#define TTY_H

#include <common.h>

#define TTY_RED 0x00FF0000
#define TTY_GREEN 0x0000FF00
#define TTY_BLUE 0x000000FF
#define TTY_WHITE 0x00FFFFFF

typedef struct {
    uint16_t x;
    uint16_t y;
    uint32_t fg;
    uint32_t bg;
    uint32_t def_fg; // default foreground
    uint32_t def_bg; // default background

    uint8_t ansi_state;
    char ansi_buffer[64];
    uint8_t ansi_buffer_pos;
} tty_t;

void tty_init();
void tty_set_fg(uint32_t color);
void tty_set_bg(uint32_t color);
void tty_set_x(uint16_t x);
void tty_set_y(uint16_t y);
void tty_putc(char c);
void tty_putc_colored(char c, int32_t fg, int32_t bg);
void tty_clear();

#endif
