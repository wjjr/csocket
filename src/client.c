/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include "log.h"
#include "cp/calc.h"

static void send_request(uint_16 a, uint_16 b) {
    int_32 result = 0;

    if (calc.add(a, b, &result)) {
        if ((a + b) != result)
            die(EXIT_FAILURE, NOERR, "%u + %u != %d", a, b, result);
        else
            log_print(NOISY, "%u + %u = %d", a, b, result);
    } else
        die(EXIT_FAILURE, errno, "Calc failed");
}

void run_client(void) {
    uint_16 a, b;

    for (;;) {
        a = (rand() % UINT16_MAX) + 1;  /* NOLINT(cert-msc30-c,cert-msc50-cpp) */
        b = (rand() % UINT16_MAX) + 1;  /* NOLINT(cert-msc30-c,cert-msc50-cpp) */

        send_request(a, b);
    }
}

uint_8 run_client_benchmark(uint_16 benchmark_num) {
    uint_16 i, a, b;
    double total_time = 0, *times = malloc(sizeof(double) * benchmark_num), min, avg, max = 0, mdev = 0;
    double n = ceil(log10(benchmark_num + 1));
    clock_t begin;

    for (i = 0; i < 10; ++i)
        send_request(20, 30);

    for (i = 0; i < benchmark_num; ++i) {
        a = (rand() % UINT16_MAX) + 1;  /* NOLINT(cert-msc30-c,cert-msc50-cpp) */
        b = (rand() % UINT16_MAX) + 1;  /* NOLINT(cert-msc30-c,cert-msc50-cpp) */

        begin = clock();
        send_request(a, b);
        total_time += (times[i] = (double) (clock() - begin) / (CLOCKS_PER_SEC / 1000000.0));

        log_print(NOISY, "%.*d: %.0f µs", (int_32) n, i + 1, times[i]);
    }

    avg = total_time / benchmark_num;
    min = times[0];

    for (i = 0; i < benchmark_num; ++i) {
        max = max > times[i] ? max : times[i];
        min = min < times[i] ? min : times[i];
        mdev += pow(times[i] - avg, 2);
    }

    mdev = sqrt(mdev / benchmark_num);

    for (i = 0; i < benchmark_num; ++i)
        printf("%.0f\n", times[i]);

    printf("min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f µs\n", min, avg, max, mdev);

    return EXIT_SUCCESS;
}
