/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_CLIENT_H
#define _CSOCKET_CLIENT_H

#include "types.h"

__attribute__((noreturn)) void run_client(const struct context *);

uint_8 run_client_benchmark(const struct context *);

#endif /* _CSOCKET_CLIENT_H */
