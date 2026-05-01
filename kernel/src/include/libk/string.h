#ifndef STRING_H
#define STRING_H

#include <common.h>

void *memset(void *dst, uint8_t c, uint64_t n);

int32_t strcmp(const char *s1, const char *s2);
int32_t strncmp(const char s1[], const char s2[], uint32_t c);

#endif
