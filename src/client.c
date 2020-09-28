/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <asm/errno.h>
#include "protobuf-c-rpc.h"
#include "log.h"

static void reply_handler(const Reply *message, void *closure_data) {
    if (message)
        log_debug(NOISY, NOERR, "Reply: %ld", message->r);

    *(bool *) closure_data = true;
}

static __always_inline uint_8 send_request(const char *const hostname, const Request *const request) {
    Service *service;
    uint8_t count = 0;
    bool is_done = false;

    service = new_service(hostname);

    while (!service_is_connected(service) && count++ < 10) {
        log_debug(DEBUG, NOERR, "Connecting...");
        dispatch_run();
    }

    if (!service_is_connected(service))
        die(EXIT_FAILURE, EHOSTDOWN, "Could not connect to the server");

    log_debug(DEBUG, NOERR, "Connected.");

    calc__sub(service, request, reply_handler, &is_done);

    while (!is_done) {
        dispatch_run();
    }

    service_destroy(service);

    return EXIT_SUCCESS;
}

void run_client(const struct context *const ctx) {
    char *hostname = malloc(16);
    Request request = REQUEST__INIT;

    sprintf(hostname, "127.0.0.1:%d", ctx->port);
    request.a = 50000;
    request.b = 10000;

    for (;;) {
        send_request(hostname, &request);
    }
}

uint_8 run_client_benchmark(const struct context *const ctx) {
    double total_time = 0, *times = malloc(sizeof(double) * ctx->benchmark_num), min = 0, avg, max = 0, mdev = 0;
    clock_t begin;
    char *hostname = malloc(16);
    Request request = REQUEST__INIT;

    sprintf(hostname, "127.0.0.1:%d", ctx->port);
    request.a = 50000;
    request.b = 10000;

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i) {
        begin = clock();
        send_request(hostname, &request);
        total_time += (times[i] = (double) (clock() - begin) / (CLOCKS_PER_SEC / 1000000.0));
        log_print(NOISY, "%.5d: %.3f µs\n", i + 1, times[i]);
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
        printf("%.3f\n", times[i]);

    printf("min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f µs\n", min, avg, max, mdev);

    return EXIT_SUCCESS;
}
