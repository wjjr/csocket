/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <arpa/inet.h>
#include <asm/socket.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "log.h"
#include "common.h"

static int_32 setup_listener(const enum protocol protocol, const uint_16 port) {
    int_32 socket_fd, optval = 1;
    struct sockaddr_in address = {
            .sin_family = AF_INET,
            .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)},
            .sin_port = htons(port)
    };

    if ((socket_fd = socket(AF_INET, protocol == TCP ? SOCK_STREAM : SOCK_DGRAM, PF_UNSPEC)) < 0)
        die(EXIT_FAILURE, errno, "Failed to create socket");

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
        die(EXIT_FAILURE, errno, "Failed to set socket option (SO_REUSEPORT)");

    if (bind(socket_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
        die(EXIT_FAILURE, errno, "Failed to bind");

    if (protocol == TCP && listen(socket_fd, 16) < 0)
        die(EXIT_FAILURE, errno, "Failed to listen");

    return socket_fd;
}

static void *handle_tcp(struct context *const ctx) {
    ssize bytes;
    struct request req;
    uint_32 result;

    if ((bytes = read(ctx->client_fd, &req, sizeof(req))) < 0) {
        log_debug(DEBUG, errno, "T%d: Failed to read", ctx->thread_index);
    } else {
        log_debug(NOISY, NOERR, "T%d: Read %td bytes", ctx->thread_index, bytes);

        result = calc(req);

        if ((bytes = write(ctx->client_fd, &result, sizeof(result))) < 0) {
            log_debug(DEBUG, errno, "T%d: Failed to write", ctx->thread_index);
        } else
            log_debug(NOISY, NOERR, "T%d: Wrote %td bytes", ctx->thread_index, bytes);
    }

    for (shutdown(ctx->client_fd, SHUT_WR), bytes = -1; bytes != 0;) {
        if ((bytes = read(ctx->client_fd, &req, sizeof(req))) < 0) {
            log_debug(DEBUG, errno, "T%d: Failed to read", ctx->thread_index);
            break;
        }
    }

    close(ctx->client_fd);
    free(ctx);

    pthread_exit(NULL);
}

static __always_inline void handle_udp(const struct context *const ctx, const int_32 socket_fd) {
    ssize bytes;
    struct request req;
    struct sockaddr_in address;
    uint_32 addr_len = sizeof(struct sockaddr_in), result;

    if ((bytes = recvfrom(socket_fd, &req, sizeof(req), 0, (struct sockaddr *) &address, &addr_len)) < 0) {
        log_debug(DEBUG, errno, "T%d: Failed to recvfrom", ctx->thread_index);
        return;
    } else
        log_debug(NOISY, NOERR, "T%d: Received %td bytes", ctx->thread_index, bytes);

    result = calc(req);

    if ((bytes = sendto(socket_fd, &result, sizeof(result), 0, (struct sockaddr *) &address, addr_len)) < 0) {
        log_debug(DEBUG, errno, "T%d: Failed to sendto", ctx->thread_index);
        return;
    } else
        log_debug(NOISY, NOERR, "T%d: Sent %td bytes", ctx->thread_index, bytes);
}

static __attribute__((noreturn)) void *accept_connections(const struct context *const ctx) {
    struct context *handle_ctx;
    int_32 client_fd;
    pthread_t thread;
    pthread_attr_t thread_attr;

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    log_debug(DEBUG, NOERR, "T%d: Listening for connections...", ctx->thread_index);

    for (;;) {
        if (ctx->protocol == TCP) {
            if ((client_fd = accept(ctx->server_fd, NULL, NULL)) < 0) {
                log_debug(DEBUG, errno, "T%d: Failed to accept", ctx->thread_index);
                continue;
            }

            handle_ctx = malloc(sizeof(struct context));
            memcpy(handle_ctx, ctx, sizeof(struct context));
            handle_ctx->client_fd = client_fd;

            pthread_create(&thread, &thread_attr, (void *(*)(void *)) handle_tcp, handle_ctx);
        } else {
            handle_udp(ctx, ctx->server_fd);
        }
    }
}

uint_8 run_server(const struct context *const ctx) {
    int_32 server_fd;
    struct context *thread_ctx;
    pthread_t *threads = calloc(ctx->threads_num, sizeof(pthread_t));

    signal(SIGPIPE, SIG_IGN);

    server_fd = setup_listener(ctx->protocol, ctx->port);

    for (uint_8 i = 0; i < ctx->threads_num; ++i) {
        thread_ctx = malloc(sizeof(struct context));
        memcpy(thread_ctx, ctx, sizeof(struct context));
        thread_ctx->thread_index = i + 1;
        thread_ctx->server_fd = server_fd;

        pthread_create(&threads[i], NULL, (void *(*)(void *)) accept_connections, thread_ctx);
    }

    for (uint_8 i = 0; i < ctx->threads_num; ++i)
        pthread_join(threads[i], NULL);

    return EXIT_SUCCESS;
}
