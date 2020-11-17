/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#define _POSIX_C_SOURCE 200112L

#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 512

struct rh_conn_ctx {
    enum protocol protocol;
    int socket_fd;
};

rh_conn_ctx *rh_client_new(const enum protocol protocol, const char *host, const uint_16 port) {
    rh_conn_ctx *conn_ctx;
    int_32 socket_fd;
    char service_port[6] = {0};
    int err;
    struct addrinfo *host_addr = NULL, hints = {
            .ai_family = AF_INET,
            .ai_socktype = protocol == TCP ? SOCK_STREAM : SOCK_DGRAM,
            .ai_protocol = PF_UNSPEC
    };
    struct timeval time = {
            .tv_sec = 5,
            .tv_usec = 0
    };

    if (sprintf(service_port, "%d", port) < 1) {
        errno = EINVAL;
        return NULL;
    }

    if ((err = getaddrinfo(host, service_port, &hints, &host_addr)) != 0) {
        switch (err) {
            case EAI_NONAME:
            case EAI_FAIL:
                errno = EADDRNOTAVAIL;
                break;
            case EAI_AGAIN:
                errno = EAGAIN;
                break;
            case EAI_FAMILY:
                errno = EAFNOSUPPORT;
                break;
            case EAI_SOCKTYPE:
            case EAI_SERVICE:
                errno = ESOCKTNOSUPPORT;
                break;
            case EAI_MEMORY:
                errno = ENOMEM;
                break;
            case EAI_SYSTEM:
                break;
            case EAI_BADFLAGS:
            case EAI_OVERFLOW:
            default:
                errno = EINVAL;
                return NULL;
        }

        return NULL;
    }

    if ((socket_fd = socket(host_addr->ai_family, host_addr->ai_socktype, PF_UNSPEC)) < 0) {
        freeaddrinfo(host_addr);
        return NULL;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) < 0) {
        freeaddrinfo(host_addr);
        return NULL;
    }

    if (connect(socket_fd, host_addr->ai_addr, host_addr->ai_addrlen) < 0) {
        freeaddrinfo(host_addr);
        return NULL;
    }

    freeaddrinfo(host_addr);

    conn_ctx = malloc(sizeof(rh_conn_ctx));
    conn_ctx->protocol = protocol;
    conn_ctx->socket_fd = socket_fd;

    return conn_ctx;
}

bool rh_send_to_server(rh_conn_ctx *const conn_ctx, const byte *const data, const usize data_size) {
    if (conn_ctx->protocol == TCP) {
        if (write(conn_ctx->socket_fd, data, data_size) != (ssize) data_size)
            return false;
        else
            return true;
    } else {
        if (sendto(conn_ctx->socket_fd, data, data_size, 0, NULL, 0) != (ssize) data_size)
            return false;
        else
            return true;
    }
}

rh_server_msg *rh_receive_from_server(rh_conn_ctx *const conn_ctx) {
    rh_server_msg *server_msg = malloc(sizeof(rh_server_msg));
    ssize data_size = -1;

    server_msg->data = malloc(BUFFER_SIZE);

    if (conn_ctx->protocol == TCP) {
        data_size = read(conn_ctx->socket_fd, server_msg->data, BUFFER_SIZE);
    } else {
        data_size = recvfrom(conn_ctx->socket_fd, server_msg->data, BUFFER_SIZE, 0, NULL, NULL);
    }

    if (data_size <= 0) {
        rh_server_msg_destroy(server_msg);

        if (data_size == 0)
            errno = ENOTCONN;

        return NULL;
    } else {
        server_msg->data_size = (usize) data_size;
        server_msg->data = realloc(server_msg->data, server_msg->data_size);

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
