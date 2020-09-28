/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <asm/errno.h>
#include "protobuf-c-rpc.h"
#include "log.h"

static void calc_add(Calc_Service *service __attribute__((unused)), const Request *input, Reply_Closure closure, void *closure_data) {
    Reply reply = REPLY__INIT;

    reply.r = input->a + input->b;
    log_debug(NOISY, NOERR, "Calc.Add(%d, %d) = %ld", input->a, input->b, reply.r);

    closure(&reply, closure_data);
}

static void calc_sub(Calc_Service *service __attribute__((unused)), const Request *input, Reply_Closure closure, void *closure_data) {
    Reply reply = REPLY__INIT;

    reply.r = input->a - input->b;
    log_debug(NOISY, NOERR, "Calc.Sub(%d, %d) = %ld", input->a, input->b, reply.r);

    closure(&reply, closure_data);
}

static void calc_mul(Calc_Service *service __attribute__((unused)), const Request *input, Reply_Closure closure, void *closure_data) {
    Reply reply = REPLY__INIT;

    reply.r = input->a * input->b;
    log_debug(NOISY, NOERR, "Calc.Mul(%d, %d) = %ld", input->a, input->b, reply.r);

    closure(&reply, closure_data);
}

static void calc_div(Calc_Service *service __attribute__((unused)), const Request *input, Reply_Closure closure, void *closure_data) {
    Reply reply = REPLY__INIT;

    reply.r = input->a / input->b;
    log_debug(NOISY, NOERR, "Calc.Div(%d, %d) = %ld", input->a, input->b, reply.r);

    closure(&reply, closure_data);
}

void run_server(const struct context *const ctx) {
    char *name = malloc(6);
    static struct _Calc_Service calc_service = CALC__INIT(calc_);
    Server *server;

    sprintf(name, "%d", ctx->port);

    server = new_server(name, (Service *) &calc_service);

    if (!server)
        die(EXIT_FAILURE, EAGAIN, "Could not run the server");

    for (;;)
        dispatch_run();
}
