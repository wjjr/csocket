/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <errno.h>
#include "rh/server.h"
#include "common.h"

void run_server(const struct context *const ctx) {
    rh_server_ctx *server_ctx;
    rh_client_msg *msg;
    int_32 result;

    if (NULL == (server_ctx = rh_server_new(ctx->protocol, ctx->port)))
        die(EXIT_FAILURE, errno, "Failed to start server");
    else
        log_print(INFO, "Server is running");

    for (;; errno = 0) {
        if (NULL != (msg = rh_receive_from_client(server_ctx))) {
            if (msg->data_size == sizeof(struct request)) {
                log_print(NOISY, "Received message with %ld bytes from client", msg->data_size);

                result = calc(*((struct request *) msg->data));

                if(!rh_send_to_client(msg->return_addr, (const byte *) &result, sizeof(result)))
                    log_error(ERROR, errno, "Failed to send message to client");
                else
                    log_print(NOISY, "Sent message with %ld bytes to client", sizeof(result));
            } else {
                log_print(ERROR, "Wrong packet size");
            }

            rh_client_msg_destroy(msg);
        } else {
            log_error(ERROR, errno, "Failed to receive message from client");
        }
    }
}
