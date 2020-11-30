/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <asm/socket.h>
#include <sys/select.h>
#include <errno.h>
#include "log.h"

#define BUFFER_SIZE 1024

struct client {
    int fd;
    uint_16 pos;
};

struct rh_server_ctx {
    enum protocol protocol;
    int server_fd;
    uint_16 clients_capacity;
    uint_16 clients_count;
    struct client **clients;
};

struct rh_client_addr {
    const rh_server_ctx *server_ctx;
    struct sockaddr_in client_address;
    uint_16 client_pos;
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
    server_ctx->clients_capacity = 10;
    server_ctx->clients_count = 0;
    server_ctx->clients = malloc(sizeof(struct client *) * server_ctx->clients_capacity);

    return server_ctx;
}

static int build_fd_set(const rh_server_ctx *const server_ctx, fd_set *const read_fds) {
    int n_fds = server_ctx->server_fd;
    FD_ZERO(read_fds);
    FD_SET(server_ctx->server_fd, read_fds);  /* NOLINT(hicpp-signed-bitwise) */

    for (uint_16 i = 0; i < server_ctx->clients_count; ++i) {
        if (server_ctx->clients[i] != NULL) {
            FD_SET(server_ctx->clients[i]->fd, read_fds);  /* NOLINT(hicpp-signed-bitwise) */

            if (server_ctx->clients[i]->fd > n_fds)
                n_fds = server_ctx->clients[i]->fd;
        }
    }

    return n_fds + 1;
}

static void accept_connection(rh_server_ctx *const server_ctx) {
    int client_fd = accept(server_ctx->server_fd, NULL, NULL);

    if (client_fd > 0) {
        struct client *client = malloc(sizeof(struct client));
        client->fd = client_fd;
        client->pos = server_ctx->clients_count;

        if (server_ctx->clients_count == server_ctx->clients_capacity) {
            server_ctx->clients_capacity *= 2;
            server_ctx->clients = realloc(server_ctx->clients, server_ctx->clients_capacity);
        }

        server_ctx->clients[server_ctx->clients_count++] = client;
    }
}

static struct client *get_client_addr(const rh_server_ctx *const server_ctx, const fd_set *const fds) {
    for (uint_16 i = 0; i < server_ctx->clients_count; ++i) {
        if (server_ctx->clients[i] != NULL && FD_ISSET(server_ctx->clients[i]->fd, fds))  /* NOLINT(hicpp-signed-bitwise) */
            return server_ctx->clients[i];
    }

    return NULL;
}

rh_client_msg *rh_receive_from_client(rh_server_ctx *const server_ctx) {
    rh_client_msg *client_msg = malloc(sizeof(rh_client_msg));
    uint_32 addr_len = sizeof(struct sockaddr_in);
    ssize data_size = -2;
    int n_fds;
    fd_set read_fds;
    struct client *client = NULL;

    client_msg->data = malloc(BUFFER_SIZE);
    client_msg->return_addr = malloc(sizeof(rh_client_addr));
    client_msg->return_addr->server_ctx = server_ctx;

    if (server_ctx->protocol == TCP) {
        n_fds = build_fd_set(server_ctx, &read_fds);

        switch (select(n_fds, &read_fds, NULL, NULL, NULL)) {
            case -1:
                die(EXIT_FAILURE, errno, "Server error: select()");
            case 0:
                break;
            default:
                if (FD_ISSET(server_ctx->server_fd, &read_fds))  /* NOLINT(hicpp-signed-bitwise) */
                    accept_connection(server_ctx);

                client = get_client_addr(server_ctx, &read_fds);
        }

        if (client != NULL) {
            client_msg->return_addr->client_pos = client->pos;

            data_size = read(client->fd, client_msg->data, 512);
        }
    } else {
        data_size = recvfrom(server_ctx->server_fd, client_msg->data, BUFFER_SIZE, 0,
                             (struct sockaddr *) &client_msg->return_addr->client_address, &addr_len);
    }

    if (data_size <= 0) {
        if (data_size == 0)
            errno = ENOTCONN;
        else if (data_size == -1)
            log_debug(DEBUG, errno, "read() error");

        rh_client_msg_destroy(client_msg, data_size != -2);

        return NULL;
    } else {
        client_msg->data_size = (usize) data_size;
        client_msg->data = realloc(client_msg->data, client_msg->data_size);

        return client_msg;
    }
}

static void close_client(const rh_client_addr *const return_addr) {
    struct client *client = return_addr->server_ctx->clients[return_addr->client_pos];

    if (client != NULL) {
        shutdown(client->fd, SHUT_WR);

        byte *buffer = malloc(BUFFER_SIZE);

        for (;;)
            if ((read(client->fd, buffer, BUFFER_SIZE)) <= 0)
                break;

        close(client->fd);
        return_addr->server_ctx->clients[client->pos] = NULL;

        free(buffer);
        free(client);
    }
}

bool rh_send_to_client(const rh_client_addr *const return_addr, const byte *const data, const usize data_size) {
    if (return_addr->server_ctx->protocol == TCP) {
        struct client *client = return_addr->server_ctx->clients[return_addr->client_pos];

        if (client && write(client->fd, data, data_size) == (ssize) data_size)
            return true;
        else {
            close_client(return_addr);

            return false;
        }
    } else {
        if (sendto(return_addr->server_ctx->server_fd, data, data_size, 0,
                   (const struct sockaddr *) &return_addr->client_address, sizeof(struct sockaddr_in)) != (ssize) data_size)
            return false;
        else
            return true;
    }
}

void rh_client_msg_destroy(rh_client_msg *client_msg, const bool do_close) {
    if (client_msg->return_addr->server_ctx->protocol == TCP && do_close) {
        close_client(client_msg->return_addr);
    }

    free(client_msg->data);
    free(client_msg->return_addr);
    free(client_msg);
}
