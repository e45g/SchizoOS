#ifndef VIDEO_H
#define VIDEO_H

#include <common.h>
#include <boot.h>

void draw_char(boot_info_t *bt, char c, int x, int y, int32_t color);
void draw_string(boot_info_t *bt, char *str, int x, int y, int32_t color);

#endif
