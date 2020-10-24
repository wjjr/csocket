/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 512

struct rh_conn_ctx {
    enum protocol protocol;
    int socket_fd;
    struct sockaddr_in address_out;
    struct sockaddr_in address_in;
};

rh_conn_ctx *rh_client_new(const enum protocol protocol, const uint_16 port) {
    rh_conn_ctx *conn_ctx;
    int_32 socket_fd;
    struct timeval time = {
            .tv_sec = 5,
            .tv_usec = 0
    };
    struct sockaddr_in address_out = {
            .sin_family = AF_INET,
            .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)},
            .sin_port = htons(port)
    };
    struct sockaddr_in address_in = {
            .sin_family = AF_INET,
            .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)},
            .sin_port = 0
    };

    if ((socket_fd = socket(AF_INET, protocol == TCP ? SOCK_STREAM : SOCK_DGRAM, PF_UNSPEC)) < 0)
        return NULL;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) < 0)
        return NULL;

    if (protocol == TCP && connect(socket_fd, (const struct sockaddr *) &address_out, sizeof(address_out)) < 0)
        return NULL;

    if (protocol == UDP && bind(socket_fd, (struct sockaddr *) &address_in, sizeof(address_in)) < 0)
        return NULL;

    conn_ctx = malloc(sizeof(rh_conn_ctx));
    conn_ctx->protocol = protocol;
    conn_ctx->socket_fd = socket_fd;
    conn_ctx->address_out = address_out;
    conn_ctx->address_in = address_in;

    return conn_ctx;
}

bool rh_send_to_server(rh_conn_ctx *const conn_ctx, const byte *const data, const ssize data_size) {
    if (conn_ctx->protocol == TCP) {
        if (write(conn_ctx->socket_fd, data, (usize) data_size) != data_size)
            return false;
        else
            return true;
    } else {
        if (sendto(conn_ctx->socket_fd, data, (usize) data_size, 0,
                   (const struct sockaddr *) &conn_ctx->address_out, sizeof(struct sockaddr_in)) != data_size)
            return false;
        else
            return true;
    }
}

rh_server_msg *rh_receive_from_server(rh_conn_ctx *const conn_ctx) {
    rh_server_msg *server_msg = malloc(sizeof(rh_server_msg));
    uint_32 addr_len = sizeof(struct sockaddr_in);

    server_msg->data = malloc(BUFFER_SIZE);
    server_msg->data_size = -1;

    if (conn_ctx->protocol == TCP) {
        server_msg->data_size = read(conn_ctx->socket_fd, server_msg->data, BUFFER_SIZE);
    } else {
        server_msg->data_size = recvfrom(conn_ctx->socket_fd, server_msg->data, BUFFER_SIZE, 0, (struct sockaddr *) &conn_ctx->address_in, &addr_len);
    }

    if (server_msg->data_size < 0) {
        rh_server_msg_destroy(server_msg);

        return NULL;
    } else {
        server_msg->data = realloc(server_msg->data, (usize) server_msg->data_size);

        return server_msg;
    }
}

void rh_server_msg_destroy(rh_server_msg *server_msg) {
    free(server_msg->data);
    free(server_msg);
}

void rh_client_destroy(rh_conn_ctx *conn_ctx) {
    close(conn_ctx->socket_fd);
    free(conn_ctx);
}
