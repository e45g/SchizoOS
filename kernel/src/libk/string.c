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
