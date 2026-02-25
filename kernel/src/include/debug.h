#ifndef DEBUG_H
#define DEBUG_H

#include <libk/stdio.h>

#define DEBUG_LEVEL 100

enum log_levels {
    LOG_FATAL,
    LOG_ERROR,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_OK,
};

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)

#define DEBUG_SHOW_SOURCE
#undef DEBUG_SHOW_SOURCE

#ifndef DEBUG_SHOW_SOURCE
    #define DEBUG_SRC ""
#else
    #define DEBUG_SRC "\x1b[38;2;13;15;18;48;2;167;162;132m" __FILE__ ":" STRINGIFY(__LINE__) "\x1b[0m"
#endif

#define TRACE(msg, ...)                                                                              \
    do {                                                                                             \
        if(DEBUG_LEVEL >= LOG_TRACE)                                                                  \
            printf(DEBUG_SRC "\x1b[38;2;13;15;18;45m trace \x1b[49;35m " msg "\x1b[0m", __FILE__, __LINE__, ##__VA_ARGS__);  \
    } while (0)

#define OK(msg, ...)                                                                              \
    do {                                                                                             \
        if(DEBUG_LEVEL >= LOG_TRACE)                                                                  \
            printf(DEBUG_SRC "\x1b[38;2;13;15;18;48;2;113;180;141m ok    \x1b[49;38;2;113;180;141m " msg "\x1b[0m", __FILE__, __LINE__, ##__VA_ARGS__);  \
    } while (0)


#endif
