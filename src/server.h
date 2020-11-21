/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_SERVER_H
#define CSOCKET_SERVER_H

#include "types/primitive.h"
#include "rh/types.h"

__attribute__((noreturn)) void run_server(enum protocol protocol, uint_16 port, uint_8 thread_num);

#endif /* CSOCKET_SERVER_H */
