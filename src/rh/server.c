/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <asm/socket.h>

#define BUFFER_SIZE 512

struct rh_server_ctx {
    enum protocol protocol;
    int server_fd;
};

struct rh_client_addr {
    const rh_server_ctx *server_ctx;
    int client_fd;
    struct sockaddr_in client_address;
};

rh_server_ctx *rh_server_new(const enum protocol protocol, const uint_16 port_to_listen) {
    rh_server_ctx *server_ctx;
    int_32 server_fd, optval = 1;
    struct sockaddr_in address = {
            .sin_family = AF_INET,
            .sin_addr = {.s_addr = htonl(INADDR_ANY)},
            .sin_port = htons(port_to_listen)
    };

    if ((server_fd = socket(AF_INET, protocol == TCP ? SOCK_STREAM : SOCK_DGRAM, PF_UNSPEC)) < 0)
        return NULL;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
        return NULL;

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
        return NULL;

    if (protocol == TCP && listen(server_fd, 16) < 0)
        return NULL;

    server_ctx = malloc(sizeof(rh_server_ctx));
    server_ctx->protocol = protocol;
    server_ctx->server_fd = server_fd;

    return server_ctx;
}

rh_client_msg *rh_receive_from_client(const rh_server_ctx *const server_ctx) {
    rh_client_msg *client_msg = malloc(sizeof(rh_client_msg));
    uint_32 addr_len = sizeof(struct sockaddr_in);

    client_msg->data = malloc(BUFFER_SIZE);
    client_msg->data_size = -1;
    client_msg->return_addr = malloc(sizeof(rh_client_addr));
    client_msg->return_addr->server_ctx = server_ctx;

    if (server_ctx->protocol == TCP) {
        client_msg->return_addr->client_fd = accept(server_ctx->server_fd, NULL, NULL);

        if (client_msg->return_addr->client_fd > 0)
            client_msg->data_size = read(client_msg->return_addr->client_fd, client_msg->data, 512);
    } else {
        client_msg->data_size = recvfrom(server_ctx->server_fd, client_msg->data, BUFFER_SIZE, 0,
                                         (struct sockaddr *) &client_msg->return_addr->client_address, &addr_len);
    }

    if (client_msg->data_size < 0) {
        rh_client_msg_destroy(client_msg);

        return NULL;
    } else {
        client_msg->data = realloc(client_msg->data, (usize) client_msg->data_size);

        return client_msg;
    }
}

bool rh_send_to_client(const rh_client_addr *const return_addr, const byte *const data, const ssize data_size) {
    if (return_addr->server_ctx->protocol == TCP) {
        if (write(return_addr->client_fd, data, (usize) data_size) != data_size)
            return false;
        else
            return true;
    } else {
        if (sendto(return_addr->server_ctx->server_fd, data, (usize) data_size, 0,
                   (const struct sockaddr *) &return_addr->client_address, sizeof(struct sockaddr_in)) != data_size)
            return false;
        else
            return true;
    }
}

void rh_client_msg_destroy(rh_client_msg *client_msg) {
    if (client_msg->return_addr->server_ctx->protocol == TCP) {
        shutdown(client_msg->return_addr->client_fd, SHUT_WR);

        byte *buffer = malloc(BUFFER_SIZE);

        for (;;)
            if ((read(client_msg->return_addr->client_fd, buffer, BUFFER_SIZE)) <= 0)
                break;

        close(client_msg->return_addr->client_fd);
        free(buffer);
    }

    free(client_msg->data);
    free(client_msg->return_addr);
    free(client_msg);
}
