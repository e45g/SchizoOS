#include <tty.h>

static void print_unsigned(uint64_t value, int base) {
    char buf[64];
    int i = 0;
    const char *digits = "0123456789abcdef";

    do {
        buf[i++] = digits[value % base];
        value /= base;
    } while (value > 0);

    while (i > 0) {
        tty_putc(buf[--i]);
    }
}

static void print_signed(int64_t value) {
    if (value < 0) {
        tty_putc('-');
        value = -value;
    }
    print_unsigned((uint64_t)value, 10);
}

static void print_hex(uint64_t value, int width) {
    for (int i = width - 1; i >= 0; i--) {
        int nibble = (value >> (i * 4)) & 0xF;
        tty_putc("0123456789abcdef"[nibble]);
    }
}


void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p != '\0'; p++) {
        if (*p == '%' && *(p + 1) != '\0') {
            p++;
            bool long_mode = false;
            if (*p == 'l') {
                long_mode = true;
                p++;
            }

            switch (*p) {
                case 's': {
                    char *s = va_arg(args, char*);
                    if (!s) s = "(null)";
                    while (*s) tty_putc(*s++);
                    break;
                }
                case 'c': {
                    tty_putc((char)va_arg(args, int));
                    break;
                }
                case 'd':
                case 'i': {
                    int64_t val = long_mode ? va_arg(args, int64_t) : (int64_t)va_arg(args, int);
                    print_signed(val);
                    break;
                }
                case 'u': {
                    uint64_t val = long_mode ? va_arg(args, uint64_t) : (uint64_t)va_arg(args, unsigned int);
                    print_unsigned(val, 10);
                    break;
                }
                case 'x': {
                    uint64_t val = long_mode ? va_arg(args, uint64_t) : (uint64_t)va_arg(args, unsigned int);
                    print_unsigned(val, 16);
                    break;
                }
                case 'p': {
                    uint64_t val = (uintptr_t)va_arg(args, void*);
                    tty_putc('0'); tty_putc('x');
                    print_hex(val, 16);
                    break;
                }
                case '%': {
                    tty_putc('%');
                    break;
                }
                default:
                    tty_putc('%');
                    tty_putc(*p);
                    break;
            }
        } else {
            tty_putc(*p);
        }
    }
    va_end(args);
}
