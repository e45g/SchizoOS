#include <boot.h>
#include <video.h>
#include <tty.h>

static tty_t tty;

static uint32_t term_width;
static uint32_t term_height;

static const uint32_t ansi_color_table[16] = {
    0x00000000, // 0  black
    0x00AA0000, // 1  red
    0x0000AA00, // 2  green
    0x00AA5500, // 3  yellow
    0x000000AA, // 4  blue
    0x008C78B4, // 5  magenta
    0x0000AAAA, // 6  cyan
    0x00d0d0d0, // 7  white
    0x00555555, // 8  bright black (gray)
    0x00FF5555, // 9  bright red
    0x0055FF55, // 10 bright green
    0x00FFFF55, // 11 bright yellow
    0x005555FF, // 12 bright blue
    0x00FF55FF, // 13 bright magenta
    0x0055FFFF, // 14 bright cyan
    0x00FFFFFF, // 15 bright white
};

static void tty_ansi_sgr() {
    tty.ansi_buffer[tty.ansi_buffer_pos] = '\0';

    uint32_t params[32];
    int nparams = 0;
    char *p = tty.ansi_buffer;

    while (*p != '\0' && nparams < 32) {
        uint32_t val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (uint32_t)(*p - '0');
            p++;
        }
        params[nparams++] = val;
        if (*p == ';') p++;
    }

    if (nparams == 0) {
        tty.fg = tty.def_fg;
        tty.bg = tty.def_bg;
        return;
    }

    int i = 0;
    while(i != nparams) {
        uint32_t v = params[i];
        if(v == 0) {
            tty.fg = tty.def_fg;
            tty.bg = tty.def_bg;
        }
        else if(v >= 30 && v <= 37) {
            tty.fg = ansi_color_table[v-30];
        }
        else if(v == 39) {
            tty.fg = tty.def_fg;
        }
        else if(v >= 40 && v <= 47) {
            tty.bg = ansi_color_table[v-40];
        }
        else if(v == 49) {
            tty.bg = tty.def_bg;
        }
        else if(v >= 90 && v <= 97) {
            tty.fg = ansi_color_table[v-90];
        }

        else if(v >= 100 && v <= 107) {
            tty.fg = ansi_color_table[v-100];
        }
        else if(v == 38 && i + 1 < nparams) {
            uint8_t mode = params[i+1];
            if(mode == 2 && i + 4 < nparams) {
                tty.fg = (params[i+2] << 16) |
                         (params[i+3] << 8) |
                         params[i+4];
            }
            i += 4;
        }

        else if(v == 48 && i + 1 < nparams) {
            uint8_t mode = params[i+1];
            if(mode == 2 && i + 4 < nparams) {
                tty.bg = (params[i+2] << 16) |
                         (params[i+3] << 8) |
                         params[i+4];
            }
            i += 4;
        }
        i++;
    }
}

static void tty_handle_ansi(char c) {
    if (tty.ansi_state == 1) {
        if (c == '[') {
            tty.ansi_state = 2;
            tty.ansi_buffer_pos = 0;
        } else {
            tty.ansi_state = 0;
        }
        return;
    }

    if (tty.ansi_state == 2) {
        // Parameter bytes: 0x30–0x3F  (digits, ;, <, =, >, ?)
        // Intermediate bytes: 0x20–0x2F  (space, !, ", …)
        // Final byte: 0x40–0x7E
        if (c >= 0x30 && c <= 0x3F) {
            if (tty.ansi_buffer_pos < (uint8_t)(sizeof(tty.ansi_buffer) - 1)) {
                tty.ansi_buffer[tty.ansi_buffer_pos++] = c;
            }
        } else if (c >= 0x40 && c <= 0x7E) {
            if (c == 'm') {
                tty_ansi_sgr();
            }
            tty.ansi_state = 0;
            tty.ansi_buffer_pos = 0;
        }
        return;
    }
}

void tty_init() {
    uint32_t fw, fh;
    fb_get_font_dims(&fw, &fh);

    term_width  = fb_get_width() / fw;
    term_height = fb_get_height() / fh;

    tty.x = 0;
    tty.y = 0;
    tty.def_fg = 0x00d0d0d0;
    tty.def_bg = 0x000d0f12;
    tty.fg = tty.def_fg;
    tty.bg = tty.def_bg;

    tty.ansi_state = 0;
    tty.ansi_buffer_pos = 0;

    tty_clear();
}

void tty_putc(char c) {
    if (tty.ansi_state != 0 || c == '\x1b') {
        if (c == '\x1b') {
            tty.ansi_state = 1;
            return;
        }
        tty_handle_ansi(c);
        return;
    }

    uint32_t fw, fh;
    fb_get_font_dims(&fw, &fh);

    if (c == '\n') {
        tty.x = 0;
        tty.y += fh;
    } else if (c == '\r') {
        tty.x = 0;
    } else if (c == '\t') {
        uint32_t col = tty.x / fw;
        uint32_t next = (col + 8) & ~7u;
        tty.x = (uint16_t)(next * fw);
    } else {
        fb_draw_char(c, tty.x, tty.y, tty.fg, tty.bg);
        tty.x += fw;
    }

    // Wrap
    if (tty.x + fw > fb_get_width()) {
        tty.x = 0;
        tty.y += fh;
    }

    // Scroll
    if (tty.y + fh > fb_get_height()) {
        fb_scroll(tty.bg);
        tty.y -= fh;
    }
}

void tty_putc_colored(char c, int32_t fg, int32_t bg) {
    uint32_t old_fg = tty.fg;
    uint32_t old_bg = tty.bg;
    if (fg >= 0) tty.fg = (uint32_t)fg;
    if (bg >= 0) tty.bg = (uint32_t)bg;
    tty_putc(c);
    tty.fg = old_fg;
    tty.bg = old_bg;
}
void tty_clear() {
    fb_clean_screen(tty.bg);
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
