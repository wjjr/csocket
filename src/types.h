/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_TYPES_H
#define _CSOCKET_TYPES_H

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>

#define PRIdSIZ PRIdPTR
#define PRIuSIZ PRIuPTR
#define USIZE_MAX SIZE_MAX
#define BYTE_MAX UCHAR_MAX
#define UINT24_MAX (16777215U)
#if USIZE_MAX > UINT32_MAX
#define USIZE_C(c) UINT64_C(c)
#else
#define USIZE_C(c) UINT32_C(c)
#endif

#define false 0
#define true 1

typedef int_least8_t int_8;
typedef int_least16_t int_16;
typedef int_least32_t int_32;
typedef int_least64_t int_64;
typedef uint_least8_t uint_8;
typedef uint_least16_t uint_16;
typedef uint_least32_t uint_32;
typedef uint_least64_t uint_64;
typedef size_t usize;
typedef ptrdiff_t ssize;
typedef uint_8 byte;
typedef uint_8 bool;

typedef struct { uint_32 _:24; } __attribute__((__packed__)) uint_24;

enum mode {
    CLIENT,
    SERVER
};

enum protocol {
    TCP,
    UDP
};

struct context {
    enum mode mode;
    uint_16 port;
    uint_16 benchmark_num;
};

enum OP {
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/'
} __attribute__ ((__packed__));

struct request {
    uint_16 a;
    uint_16 b;
    enum OP op;
} __attribute__ ((__packed__));

#endif /* _CSOCKET_TYPES_H */
