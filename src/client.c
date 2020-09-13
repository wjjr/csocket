/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "log.h"
#include "common.h"

static __always_inline struct request random_request() {
    struct request req = {
            .a = htons((rand() % UINT16_MAX) + 1),  // NOLINT(cert-msc30-c,cert-msc50-cpp)
            .b = htons((rand() % UINT16_MAX) + 1)   // NOLINT(cert-msc30-c,cert-msc50-cpp)
    };

    switch (rand() % 4) {  // NOLINT(cert-msc30-c,cert-msc50-cpp)
        case 0:
            req.op = ADD;
            break;
        case 1:
            req.op = SUB;
            break;
        case 2:
            req.op = MUL;
            break;
        case 3:
            req.op = DIV;
            break;
    }

    return req;
}

static __always_inline uint_8 send_request(const struct context *const ctx) {
    int_32 socket_fd;
    struct timeval time = {
            .tv_sec = 1,
            .tv_usec = 0
    };
    struct sockaddr_in address_out = {
            .sin_family = AF_INET,
            .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)},
            .sin_port = htons(ctx->port)
    };
    struct sockaddr_in address_in = {
            .sin_family = AF_INET,
            .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)},
            .sin_port = 0
    };
    struct request req;
    ssize bytes;
    uint_32 result, addr_len = sizeof(struct sockaddr_in);

    if ((socket_fd = socket(AF_INET, ctx->protocol == TCP ? SOCK_STREAM : SOCK_DGRAM, PF_UNSPEC)) < 0)
        die(EXIT_FAILURE, errno, "Failed to create socket");

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) < 0)
        die(EXIT_FAILURE, errno, "Failed to set socket option (SO_RCVTIMEO)");

    if (ctx->protocol == TCP && connect(socket_fd, (const struct sockaddr *) &address_out, sizeof(address_out)) < 0)
        die(EXIT_FAILURE, errno, "Failed to create socket");

    if (ctx->protocol == UDP && bind(socket_fd, (struct sockaddr *) &address_in, sizeof(address_in)) < 0)
        die(EXIT_FAILURE, errno, "Failed to bind");

    req = random_request();

    if (ctx->protocol == TCP) {
        if ((bytes = write(socket_fd, &req, sizeof(req))) < 0)
            die(EXIT_FAILURE, errno, "Failed to write");
        else
            log_debug(NOISY, NOERR, "Wrote %td bytes", bytes);

        if ((bytes = read(socket_fd, &result, sizeof(result))) < 0)
            die(EXIT_FAILURE, errno, "Failed to read");
        else
            log_debug(NOISY, NOERR, "Read %td bytes", bytes);
    } else {
        if ((bytes = sendto(socket_fd, &req, sizeof(req), 0, (const struct sockaddr *) &address_out, sizeof(address_out))) < 0)
            die(EXIT_FAILURE, errno, "Failed to sendto");
        else
            log_debug(NOISY, NOERR, "Sent %td bytes", bytes);

        if ((bytes = recvfrom(socket_fd, &result, sizeof(result), 0, (struct sockaddr *) &address_in, &addr_len)) < 0)
            die(EXIT_FAILURE, errno, "Failed to recvfrom");
        else
            log_debug(NOISY, NOERR, "Received %td bytes", bytes);
    }

    close(socket_fd);

#ifdef __C_DEBUG
    if (calc(req) != result)
        die(EXIT_FAILURE, NOERR, "%u %c %u != %u", req.a, req.op, req.b, result);
#endif

    return EXIT_SUCCESS;
}

uint_8 run_client(const struct context *const ctx) {
    for (;;) {
        send_request(ctx);
    }
}

uint_8 run_client_benchmark(const struct context *const ctx) {
    double total_time = 0, *times = malloc(sizeof(double) * ctx->benchmark_num), avg, sd = 0;
    clock_t begin;

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i) {
        begin = clock();
        send_request(ctx);
        total_time += (times[i] = (double) (clock() - begin) / (CLOCKS_PER_SEC / 1000000.0));
        log_print(NOISY, "%.5d: %.3f µs\n", i + 1, times[i]);
    }

    avg = total_time / ctx->benchmark_num;

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i)
        sd += pow(times[i] - avg, 2);

    sd = sqrt(sd / ctx->benchmark_num);

    printf("Average response time: %.3f µs\nStandard deviation: %.3f µs\n", avg, sd);

    return EXIT_SUCCESS;
}
