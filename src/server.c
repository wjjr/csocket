/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <errno.h>
#include "log.h"
#include "rh/server.h"

void run_server(const enum protocol protocol, const uint_16 port) {
    rh_server_ctx *server_ctx;
    rh_client_msg *msg;

    if (NULL == (server_ctx = rh_server_new(protocol, port)))
        die(EXIT_FAILURE, errno, "Failed to start server");
    else
        log_print(INFO, "Server is running");

    for (;; errno = 0) {
        if (NULL != (msg = rh_receive_from_client(server_ctx))) {
            log_print(NOISY, "Received message with %ld bytes from client", msg->data_size);

            /* TODO */

            if (!rh_send_to_client(msg->return_addr, (const byte *) "I\4\1\0\0\0", 6))
                log_error(ERROR, errno, "Failed to send message to client");
            else
                log_print(NOISY, "Sent message with %d bytes to client", 4);

            rh_client_msg_destroy(msg);
        } else {
            log_error(ERROR, errno, "Failed to receive message from client");
        }
    }
}
