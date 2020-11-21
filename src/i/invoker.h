/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_INVOKER_H
#define CSOCKET_INVOKER_H

#include "types/primitive.h"
#include "rh/types.h"
#include "service.h"

struct invoker;

struct invoker *invoker_new(enum protocol, uint_16 port, uint_8 threads_num);

__attribute__((noreturn)) void invoker_run(struct invoker *, const struct service *);

#endif /* CSOCKET_INVOKER_H */
