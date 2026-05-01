#include <common.h>

void *memset(void *dst, uint8_t c, uint64_t n) {
    unsigned char *p = dst;

    while(n > 0) {
        *p = c;
        p++;
        n--;
    }
    return dst;
}
int32_t strcmp(const char *s1, const char *s2) {
    while(*s1 == *s2) {
        if(*s1 == '\0') {
            return 0;
        }
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int32_t strncmp(const char *s1, const char *s2, uint32_t c) {
    if (c == 0) return 0;

    while (c-- && *s1 == *s2) {
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }

    if (c == (uint32_t)-1)
        return 0;

    return (unsigned char)*s1 - (unsigned char)*s2;
}
