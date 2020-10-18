/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_KAFKA_H
#define _CSOCKET_KAFKA_H

#include <librdkafka/rdkafka.h>

void kafka_start_producer_pooling(rd_kafka_t *rk);

#endif /* _CSOCKET_KAFKA_H */
