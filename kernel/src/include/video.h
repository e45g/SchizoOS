#ifndef VIDEO_H
#define VIDEO_H

#include <common.h>
#include <boot.h>

void clean_screen(int32_t color);
void draw_char( char c, int x, int y, int32_t color);
void draw_string(char *str, int x, int y, int32_t color);

#endif
