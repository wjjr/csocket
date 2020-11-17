/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_TYPES_PRIMITIVE_H
#define CSOCKET_TYPES_PRIMITIVE_H

#include <stdint.h>
#include <stddef.h>

typedef int_least8_t int_8;
typedef int_least16_t int_16;
typedef int_least32_t int_32;
typedef int_least64_t int_64;
typedef uint_least8_t uint_8;
typedef uint_least16_t uint_16;
typedef uint_least32_t uint_32;
typedef uint_least64_t uint_64;
typedef unsigned char byte;
typedef unsigned long usize;
typedef signed long ssize;
typedef _Bool bool;

#define false 0
#define true 1

#endif /* CSOCKET_TYPES_PRIMITIVE_H */
