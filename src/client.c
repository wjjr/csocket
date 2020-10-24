/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <errno.h>
#include "log.h"
#include "common.h"
#include "rh/client.h"

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

static __always_inline uint_8 send_request(const struct context *const ctx, const struct request request) {
    rh_conn_ctx *conn_ctx;
    rh_server_msg *msg;
    int_32 result;
    errno = 0;

    if (NULL == (conn_ctx = rh_client_new(ctx->protocol, ctx->port)))
        die(EXIT_FAILURE, errno, "Failed to create connection");

    if (rh_send_to_server(conn_ctx, (const byte *) &request, sizeof(request))) {
        log_print(NOISY, "Sent message with %ld bytes to server", sizeof(request));

        if (NULL != (msg = rh_receive_from_server(conn_ctx))) {
            log_print(NOISY, "Received message with %ld bytes from server", msg->data_size);

            if (msg->data_size == sizeof(uint_32)) {
                memcpy(&result, msg->data, (usize) msg->data_size);

#ifdef _CSOCKET_DEBUG
                if (calc(request) != result)
                    die(EXIT_FAILURE, NOERR, "%u %c %u != %d", ntohs(request.a), request.op, ntohs(request.b), (int_32) ntohl((uint_32) result));
                else
                    log_print(NOISY, "%u %c %u = %d", ntohs(request.a), request.op, ntohs(request.b), (int_32) ntohl((uint_32) result));
#endif

                rh_server_msg_destroy(msg);
                rh_client_destroy(conn_ctx);
            } else {
                die(EXIT_FAILURE, NOERR, "Wrong packet size");
            }
        } else {
            die(EXIT_FAILURE, errno, "Failed to receive message from server");
        }
    } else {
        die(EXIT_FAILURE, errno, "Failed to send message to server");
    }

    return EXIT_SUCCESS;
}

void run_client(const struct context *const ctx) {
    for (;;)
        send_request(ctx, random_request());
}

uint_8 run_client_benchmark(const struct context *const ctx) {
    double total_time = 0, *times = malloc(sizeof(double) * ctx->benchmark_num), min, avg, max = 0, mdev = 0;
    double n = ceil(log10(ctx->benchmark_num + 1));
    clock_t begin;
    struct request request;

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i) {
        request = random_request();
        request.op = ADD;

        begin = clock();
        send_request(ctx, request);
        total_time += (times[i] = (double) (clock() - begin) / (CLOCKS_PER_SEC / 1000000.0));
        log_print(NOISY, "%.*d: %.0f µs", (int_32) n, i + 1, times[i]);
    }

    avg = total_time / ctx->benchmark_num;
    min = times[0];

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i) {
        max = max > times[i] ? max : times[i];
        min = min < times[i] ? min : times[i];
        mdev += pow(times[i] - avg, 2);
    }

    mdev = sqrt(mdev / ctx->benchmark_num);

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i)
        printf("%.0f\n", times[i]);

    printf("min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f µs\n", min, avg, max, mdev);

    return EXIT_SUCCESS;
}
