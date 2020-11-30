/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "invoker.h"

#include <stdlib.h>
#include <thpool/thpool.h>
#include <errno.h>
#include <m/marshaller.h>
#include "rh/server.h"
#include "log.h"

struct invoker {
    enum protocol protocol;
    uint_16 port;
    threadpool thpool;
    uint_8 threads_num;
    const struct service *service;
};

struct req {
    const struct invoker *invoker;
    rh_client_msg *msg;
};

struct invoker *invoker_new(const enum protocol protocol, const uint_16 port, const uint_8 threads_num) {
    struct invoker *invoker = malloc(sizeof(struct invoker));

    invoker->protocol = protocol;
    invoker->port = port;
    invoker->threads_num = threads_num;
    invoker->thpool = thpool_init(threads_num * 2);
    invoker->service = NULL;

    return invoker;
}

static void process_req(struct req *req) {
    struct value bytes_value = {0};
    char *service_name = NULL, *method = NULL;
    struct data *request = NULL, *reply;
    void (*func)(const data *, data *) = NULL;

    bytes_value.type = BYTES;
    bytes_value.size = req->msg->data_size;
    bytes_value.value = req->msg->data;

    unmarshall(&bytes_value, &service_name, &method, &request);

    if (request != NULL && service_name != NULL && method != NULL) {
        service_get_method(req->invoker->service, method, &func);

        if (func != NULL) {
            log_print(NOISY, "Received message with %ld bytes from client", bytes_value.size);

            reply = data_new(1);

            func(request, reply);

            if (data_size(reply) > 0) {
                bytes_value.size = 0;
                bytes_value.value = NULL;

                marshall(reply, NULL, NULL, &bytes_value);

                if (bytes_value.size > 0 && rh_send_to_client(req->msg->return_addr, bytes_value.value, bytes_value.size)) {
                    log_print(NOISY, "Sent message with %ld bytes to client", bytes_value.size);
                }

                marshall_free(&bytes_value);
            }

            data_destroy(reply);
        }

        unmarshall_free(&service_name, &method, &request);
    }

    rh_client_msg_destroy(req->msg, false);
    free(req);
}

static __attribute__((noreturn)) void run_server(const struct invoker *const invoker) {
    rh_server_ctx *server_ctx;
    rh_client_msg *msg;
    struct req *req = NULL;

    if (NULL == (server_ctx = rh_server_new(invoker->protocol, invoker->port)))
        die(EXIT_FAILURE, errno, "Failed to start server");
    else
        log_print(INFO, "Server is running");

    for (;; errno = 0) {
        if (req == NULL)
            req = malloc(sizeof(struct req));

        if (NULL != (msg = rh_receive_from_client(server_ctx))) {
            req->msg = msg;
            req->invoker = invoker;

            thpool_add_work(invoker->thpool, (void (*)(void *)) process_req, req);
            req = NULL;
        }
    }
}

void invoker_run(struct invoker *const invoker, const struct service *const service) {
    invoker->service = service;

    for (uint_8 i = 0; i < invoker->threads_num; ++i)
        thpool_add_work(invoker->thpool, (void (*)(void *)) run_server, invoker);

    thpool_wait(invoker->thpool);
    thpool_destroy(invoker->thpool);

    die(EXIT_FAILURE, NOERR, "Server died");
}
