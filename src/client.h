/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_CLIENT_H
#define CSOCKET_CLIENT_H

#include "types.h"

__attribute__((noreturn)) void run_client(const struct context *);

uint_8 run_client_benchmark(const struct context *);

#endif /* CSOCKET_CLIENT_H */
