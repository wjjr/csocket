/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_COMMON_H
#define CSOCKET_COMMON_H

#include <arpa/inet.h>
#include "log.h"

static __UNUSED int_32 calc(const struct request r) {
    switch (r.op) {
        case ADD:
            return (int_32) htonl(ntohs(r.a) + ntohs(r.b));
        case SUB:
            return (int_32) htonl(ntohs(r.a) - ntohs(r.b));
        case MUL:
            return (int_32) htonl(ntohs(r.a) * ntohs(r.b));
        case DIV:
            return (int_32) htonl(ntohs(r.a) / ntohs(r.b));
    }

    log_debug(ERROR, NOERR, "Wrong operation");

    return (int_32) htonl((uint_32) INT32_MIN);
}

#endif /* CSOCKET_COMMON_H */
