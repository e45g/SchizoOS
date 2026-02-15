#include <tty.h>

static void print_unsigned(unsigned int value) {
    if (value == 0) {
        tty_putc('0');
        return;
    }
    unsigned int div = 1;
    while (value / div > 9) div *= 10;
    while (div) {
        tty_putc('0' + (value / div));
        value %= div;
        div /= 10;
    }
}

static void print_signed(long value) {
    if (value < 0) {
        tty_putc('-');
        if (value == INT_MIN) {
            unsigned long u = (unsigned long)(-(value + 1)) + 1;
            print_unsigned(u);
            return;
        }
        value = -value;
    }
    print_unsigned((unsigned long)value);
}

static void print_hex(long value, int digits) {
    for(int i = digits-1; i >= 0; i--) {
        unsigned long nibble = (value >> (i*4)) & 0xF;
        if(i == 0 || nibble) {
            tty_putc("0123456789abcdef"[nibble]);
        }
    }
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p != '\0'; p++) {
        if(*p == '%' && *(p + 1) != '\0') {
            p++;
            bool long_mode = false;
            if(*p == 'l') {
                long_mode = true;
                p++;
            }
            switch(*p) {
                case 's': {
                    char *s = va_arg(args, char*);
                    while (*s) tty_putc(*s++);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    tty_putc(c);
                    break;
                }
                case 'i':
                case 'd': {
                    if(long_mode) {
                        long d = va_arg(args, long);
                        print_signed(d);
                    }
                    else {
                        int d = va_arg(args, int);
                        print_signed((long)d);
                    }
                    break;
                }

                case 'u': {
                    if(long_mode) {
                        unsigned long d = va_arg(args, unsigned long);
                        print_unsigned(d);
                    }
                    else {
                        unsigned int d = va_arg(args, unsigned int);
                        print_unsigned((unsigned long)d);
                    }
                    break;
                }

                case 'x': {
                    tty_putc('0');
                    tty_putc('x');
                    if (long_mode) {
                        unsigned long value = va_arg(args, unsigned long);
                        print_hex(value, sizeof(unsigned long) * 2);
                    } else {
                        unsigned int value = va_arg(args, unsigned int);
                        print_hex((unsigned long)value, sizeof(unsigned int) * 2);
                    }
                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void *);
                    unsigned long value = (unsigned long)ptr;

                    tty_putc('0');
                    tty_putc('x');
                    for (int i = 7; i >= 0; i--) {
                        int nibble = (value >> (i * 4)) & 0xF;
                        tty_putc("0123456789abcdef"[nibble]);
                    }
                    break;
                }
            }
        }
        else{
            tty_putc(*p);
        }
    }
    va_end(args);
}
