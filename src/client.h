/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_CLIENT_H
#define CSOCKET_CLIENT_H

#include "types/primitive.h"

__attribute__((noreturn)) void run_client(void);

uint_8 run_client_benchmark(uint_16 benchmark_num);

#endif /* CSOCKET_CLIENT_H */
