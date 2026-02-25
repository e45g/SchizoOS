#ifndef COMMON_H
#define COMMON_H

#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg

#define NULL ((void*)0)

#define INT_MAX __INT_MAX__
#define INT_MIN (-__INT_MAX__  -1)
#define LLONG_MIN (-__LONG_LONG_MAX__-1LL)
#define LLONG_MAX __LONG_LONG_MAX__
#define LONG_MIN (-__LONG_MAX__ -1)

#if __SIZEOF_POINTER__ == 4
typedef unsigned int uintptr_t;  // 32-bit pointer
#elif __SIZEOF_POINTER__ == 8
typedef unsigned long uintptr_t; // 64-bit pointer
#else
#error "Unsupported pointer size"
#endif

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long int64_t;
typedef unsigned long uint64_t;

#define bool _Bool
#define true 1
#define false 0

#endif
