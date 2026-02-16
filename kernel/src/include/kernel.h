#ifndef KERNEL_H
#define KERNEL_H

#include <common.h>
#include <libk/stdio.h>

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

extern uintptr_t _kernel_start;
extern uintptr_t _kernel_end;

#endif
