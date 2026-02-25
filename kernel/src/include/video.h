#ifndef VIDEO_H
#define VIDEO_H

#include <common.h>
#include <boot.h>

void fb_init(framebuffer_t *fb);
void fb_draw_char(char c, int x, int y, int32_t fg, int32_t bg);
void fb_draw_string(char *str, int x, int y, int32_t fg, int32_t bg);
void fb_clean_screen(int32_t color);
void fb_scroll(uint32_t bg_color);

uint32_t fb_get_width();
uint32_t fb_get_height();
void fb_get_font_dims(uint32_t* width, uint32_t* height);

#endif
