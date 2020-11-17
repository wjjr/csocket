/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_CLIENT_PROXY_CALC_H
#define CSOCKET_CLIENT_PROXY_CALC_H

#include "types/primitive.h"

bool calc_add(uint_16 a, uint_16 b, int_32 *result);
bool calc_sub(uint_16 a, uint_16 b, int_32 *result);
bool calc_mul(uint_16 a, uint_16 b, int_32 *result);
bool calc_div(uint_16 a, uint_16 b, int_32 *result);

static struct {
    bool (*add)(uint_16, uint_16, int_32 *);
    bool (*sub)(uint_16, uint_16, int_32 *);
    bool (*mul)(uint_16, uint_16, int_32 *);
    bool (*div)(uint_16, uint_16, int_32 *);
} __attribute__((unused)) calc = {
        .add = &calc_add,
        .sub = &calc_sub,
        .mul = &calc_mul,
        .div = &calc_div,
};

#endif /* CSOCKET_CLIENT_PROXY_CALC_H */
