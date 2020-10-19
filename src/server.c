/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include "log.h"
#include "common.h"
#include "kafka.h"

static bool rebalanced = false;

void run_server(const struct context *const ctx __UNUSED) {
    const char *broker_id = "127.0.0.1:9092",
            *group_id = "server",
            *consumer_topic = "request",
            *producer_topic;
    rd_kafka_conf_t *rkc = rd_kafka_conf_new();
    rd_kafka_t *rk_c, *rk_p;
    rd_kafka_resp_err_t r_err;
    rd_kafka_message_t *rkm = NULL;
    rd_kafka_headers_t *rkm_headers;
    size_t header_value_size;
    char err_str[512];
    int_32 reply;

    rd_kafka_conf_set_rebalance_cb(rkc, rebalance_callback);
    rd_kafka_conf_set_opaque(rkc, &rebalanced);

    if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(rkc, "bootstrap.servers", broker_id, err_str, sizeof(err_str)))
        die(EXIT_FAILURE, NOERR, "Failed to set bootstrap.servers: %s", err_str);

    if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(rkc, "group.id", group_id, err_str, sizeof(err_str)))
        die(EXIT_FAILURE, NOERR, "Failed to set group.id: %s", err_str);

    if (NULL == (rk_p = rd_kafka_new(RD_KAFKA_PRODUCER, rd_kafka_conf_dup(rkc), err_str, sizeof(err_str))))
        die(EXIT_FAILURE, NOERR, "Failed to create new producer: %s", err_str);
    else
        kafka_start_producer_polling(rk_p);

    if (NULL == (rk_c = rd_kafka_new(RD_KAFKA_CONSUMER, rkc, err_str, sizeof(err_str))))
        die(EXIT_FAILURE, NOERR, "Failed to create new consumer: %s", err_str);
    else {
        rkc = NULL;
        rd_kafka_poll_set_consumer(rk_c);
    }

    if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = kafka_subscribe(rk_c, consumer_topic, RD_KAFKA_PARTITION_UA)))
        die(EXIT_FAILURE, NOERR, "Failed to subscribe to %s: %s", consumer_topic, rd_kafka_err2str(r_err));

    while (!rebalanced)
        kafka_consumer_poll(rk_c, 200);

    log_print(INFO, "Started");

    for (;;) {
        if (NULL == (rkm = kafka_consumer_poll(rk_c, 200)))
            continue;

        if (RD_KAFKA_RESP_ERR_NO_ERROR != rkm->err) {
            log_print(ERROR, "Failed to consume from \"%s\": %s", consumer_topic, rd_kafka_message_errstr(rkm));
        } else {
            log_print(NOISY, "Message %"PRId64" received from topic \"%s\"", rkm->offset, rd_kafka_topic_name(rkm->rkt));

            if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = rd_kafka_message_headers(rkm, &rkm_headers))) {
                log_print(ERROR, "Failed to get message header: %s", rd_kafka_err2str(r_err));
            } else if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = rd_kafka_header_get_last(rkm_headers, "REPLY_TOPIC", (const void **) &producer_topic, &header_value_size))) {
                log_print(ERROR, "Failed to read message header: %s", rd_kafka_err2str(r_err));
            } else if (rkm->payload && rkm->len == sizeof(struct request)) {
                reply = calc(*((struct request *) rkm->payload));

                r_err = rd_kafka_producev(rk_p,
                                          RD_KAFKA_VTYPE_TOPIC, producer_topic,
                                          RD_KAFKA_VTYPE_MSGFLAGS, RD_KAFKA_MSG_F_COPY,
                                          RD_KAFKA_VTYPE_VALUE, &reply, sizeof(reply),
                                          RD_KAFKA_VTYPE_END
                );

                if (RD_KAFKA_RESP_ERR_NO_ERROR != r_err) {
                    log_print(ERROR, "Failed to delivery message to topic \"%s\": %s", producer_topic, rd_kafka_err2str(r_err));
                } else {
                    log_print(NOISY, "Enqueued message for topic \"%s\"", producer_topic);
                }
            }
        }

        rd_kafka_message_destroy(rkm);
    }
}
