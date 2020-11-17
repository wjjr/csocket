/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "requestor.h"

#include <stdlib.h>
#include <errno.h>
#include "log.h"
#include "rh/client.h"
#include "m/marshaller.h"

struct requestor {
    const struct host_addr *host_addr;
    bool closed;
    rh_conn_ctx *conn_ctx;
};

struct requestor *requestor_new(const struct host_addr *const host_addr) {
    rh_conn_ctx *conn_ctx;
    struct requestor *requestor;

    if (NULL == (conn_ctx = rh_client_new(host_addr->protocol, host_addr->address, host_addr->port)))
        return NULL;

    requestor = malloc(sizeof(struct requestor));

    requestor->host_addr = host_addr;
    requestor->conn_ctx = conn_ctx;
    requestor->closed = false;

    return requestor;
}

void requestor_destroy(struct requestor *const requestor) {
    rh_client_destroy(requestor->conn_ctx);
    free(requestor);
}

bool requestor_is_active(struct requestor *requestor) {
    return !requestor->closed;
}

bool requestor_invoke(struct requestor *requestor, const char *const method, const struct data *const request, struct data **const reply) {
    struct value bytes_value = {0};
    rh_server_msg *msg;

    marshall(request, requestor->host_addr->service_name, method, &bytes_value);

    if (bytes_value.size > 0 && rh_send_to_server(requestor->conn_ctx, bytes_value.value, bytes_value.size)) {
        free(bytes_value.value);

        log_print(NOISY, "Sent message with %ld bytes to server", bytes_value.size);

        if (NULL != (msg = rh_receive_from_server(requestor->conn_ctx))) {
            log_print(NOISY, "Received message with %ld bytes from server", msg->data_size);

            bytes_value.type = BYTES;
            bytes_value.size = msg->data_size;
            bytes_value.value = msg->data;

            unmarshall(&bytes_value, NULL, NULL, reply);

            rh_server_msg_destroy(msg);

            if (*reply)
                return true;
            else
                errno = ENOMSG;
        } else {
            requestor->closed = true;
            log_debug(DEBUG, errno, "Failed to receive message from server");
        }
    } else {
        requestor->closed = true;
        free(bytes_value.value);

        log_debug(DEBUG, errno, "Failed to send message to server");
    }

    return false;
}
