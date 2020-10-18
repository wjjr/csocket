/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "kafka.h"

#include <pthread.h>

static __attribute__((noreturn)) void *_producer_pooling(rd_kafka_t *const rk) {
    for (;;)
        rd_kafka_poll(rk, 1000);
}

void kafka_start_producer_pooling(rd_kafka_t *const rk) {
    if (RD_KAFKA_PRODUCER == rd_kafka_type(rk)) {
        pthread_t pooling_thread;

        pthread_create(&pooling_thread, NULL, (void *(*)(void *)) _producer_pooling, rk);
    }
}
