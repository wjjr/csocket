/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "kafka.h"

#include <pthread.h>
#include "log.h"

static __attribute__((noreturn)) void *_producer_polling(rd_kafka_t *const rk) {
    log_debug(DEBUG, NOERR, "Started producer polling");

    for (;;)
        rd_kafka_poll(rk, 5000);
}

void kafka_start_producer_polling(rd_kafka_t *rk) {
    if (RD_KAFKA_PRODUCER == rd_kafka_type(rk)) {
        pthread_t polling_thread;

        pthread_create(&polling_thread, NULL, (void *(*)(void *)) _producer_polling, rk);
    }
}

rd_kafka_message_t *kafka_consumer_poll(rd_kafka_t *rk, int timeout_ms) {
    rd_kafka_message_t *rkm;

    if (NULL != (rkm = rd_kafka_consumer_poll(rk, timeout_ms)) && RD_KAFKA_RESP_ERR_NO_ERROR != rkm->err)
        log_print(ERROR, "Consumer poll: %s", rd_kafka_message_errstr(rkm));

    return rkm;
}

rd_kafka_resp_err_t kafka_subscribe(rd_kafka_t *rk, const char *topic_name, int32_t partition) {
    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t *subscription_list = rd_kafka_topic_partition_list_new(1);

    rd_kafka_topic_partition_list_add(subscription_list, topic_name, partition);

    if (RD_KAFKA_RESP_ERR_NO_ERROR == (err = rd_kafka_subscribe(rk, subscription_list)))
        rd_kafka_topic_partition_list_destroy(subscription_list);

    return err;
}
