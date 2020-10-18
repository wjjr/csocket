/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_KAFKA_H
#define _CSOCKET_KAFKA_H

#include <librdkafka/rdkafka.h>

void kafka_start_producer_polling(rd_kafka_t *rk);

rd_kafka_message_t *kafka_consumer_poll(rd_kafka_t *rk, int timeout_ms);

rd_kafka_resp_err_t kafka_subscribe(rd_kafka_t *rk, const char *topic_name, int32_t partition);

#endif /* _CSOCKET_KAFKA_H */
