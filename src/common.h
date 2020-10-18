/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_COMMON_H
#define _CSOCKET_COMMON_H

static int_32 calc(const struct request r) {
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

#endif /* _CSOCKET_COMMON_H */
