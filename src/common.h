/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_COMMON_H
#define _CSOCKET_COMMON_H

#include "types.h"

#include <arpa/inet.h>

static __always_inline uint_32 calc(const struct request req) {
    switch (req.op) {
        case ADD:
            return ntohs(req.a) + ntohs(req.b);
        case SUB:
            return ntohs(req.a) - ntohs(req.b);
        case MUL:
            return ntohs(req.a) * ntohs(req.b);
        case DIV:
            return ntohs(req.a) / ntohs(req.b);
    }

    return 0;
}

#endif /* _CSOCKET_COMMON_H */
