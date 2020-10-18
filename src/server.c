/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include "log.h"
#include "common.h"
#include "kafka.h"

static void rebalance_callback(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *partitions, void *_ __UNUSED) {
    int i;

    switch (err) {
        case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
            for (i = 0; i < partitions->cnt; ++i)
                log_debug(INFO, NOERR, "Assign partition: Topic=\"%s\", Partition=%d, Offset=%ld",
                          partitions->elems[i].topic, partitions->elems[i].partition, partitions->elems[i].offset = RD_KAFKA_OFFSET_END);

            rd_kafka_assign(rk, partitions);
            break;
        case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
            for (i = 0; i < partitions->cnt; ++i) {
                log_debug(INFO, NOERR, "Revoke partition: Topic=\"%s\", Partition=%d, Offset=%ld",
                          partitions->elems[i].topic, partitions->elems[i].partition, partitions->elems[i].offset);
            }
            rd_kafka_assign(rk, NULL);
            break;
        default:
            rd_kafka_assign(rk, NULL);
    }
}

void run_server(const struct context *const ctx __UNUSED) {
    const char *broker_id = "127.0.0.1:9092",
            *group_id = "server",
            *consumer_topic = "request",
            *producer_topic = "reply";
    rd_kafka_conf_t *rkc = rd_kafka_conf_new();
    rd_kafka_t *rk_c, *rk_p;
    rd_kafka_topic_t *rkt_p;
    rd_kafka_topic_partition_list_t *subscription_list = rd_kafka_topic_partition_list_new(1);
    rd_kafka_resp_err_t r_err;
    rd_kafka_message_t *rkm = NULL;
    int_32 reply_partition;
    rd_kafka_headers_t *rkm_headers;
    const void *header_value;
    size_t header_value_size;
    char err_str[512];
    struct request request;
    int_32 reply;

    if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(rkc, "bootstrap.servers", broker_id, err_str, sizeof(err_str)))
        die(EXIT_FAILURE, NOERR, "Failed to set bootstrap.servers: %s", err_str);

    if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(rkc, "group.id", group_id, err_str, sizeof(err_str)))
        die(EXIT_FAILURE, NOERR, "Failed to set group.id: %s", err_str);

    if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(rkc, "auto.offset.reset", "latest", err_str, sizeof(err_str)))
        die(EXIT_FAILURE, NOERR, "Failed to set auto.offset.reset: %s", err_str);

    if (NULL == (rk_p = rd_kafka_new(RD_KAFKA_PRODUCER, rd_kafka_conf_dup(rkc), err_str, sizeof(err_str))))
        die(EXIT_FAILURE, NOERR, "Failed to create new producer: %s", err_str);
    else
        kafka_start_producer_pooling(rk_p);

    rd_kafka_conf_set_rebalance_cb(rkc, rebalance_callback);

    if (NULL == (rk_c = rd_kafka_new(RD_KAFKA_CONSUMER, rkc, err_str, sizeof(err_str))))
        die(EXIT_FAILURE, NOERR, "Failed to create new consumer: %s", err_str);
    else {
        rd_kafka_poll_set_consumer(rk_c);
        rd_kafka_topic_partition_list_add(subscription_list, consumer_topic, RD_KAFKA_PARTITION_UA);
        rkc = NULL;
    }

    if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = rd_kafka_subscribe(rk_c, subscription_list)))
        die(EXIT_FAILURE, NOERR, "Failed to subscribe to %s: %s", consumer_topic, rd_kafka_err2str(r_err));
    else
        rd_kafka_topic_partition_list_destroy(subscription_list);

    if (NULL == (rkt_p = rd_kafka_topic_new(rk_p, producer_topic, NULL)))
        die(EXIT_FAILURE, NOERR, "Failed to create new topic: %s", rd_kafka_err2str(rd_kafka_last_error()));

    log_print(INFO, "Started");

    for (;;) {
        if (NULL == (rkm = rd_kafka_consumer_poll(rk_c, 200)))
            continue;

        if (RD_KAFKA_RESP_ERR_NO_ERROR != rkm->err)
            log_print(ERROR, "Failed to consume from \"%s\": %s", consumer_topic, rd_kafka_message_errstr(rkm));
        else
            log_debug(NOISY, NOERR, "Message %"PRId64" received from topic \"%s\", partition %d", rkm->offset, rd_kafka_topic_name(rkm->rkt), rkm->partition);

        if (rkm->payload && rkm->len == sizeof(struct request)) {
            request = *((struct request *) rkm->payload);

            if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = rd_kafka_message_headers(rkm, &rkm_headers))) {
                log_print(ERROR, "Failed to get message header: %s", rd_kafka_err2str(r_err));
            } else {
                reply = calc(request);

                if (RD_KAFKA_CONF_OK != (r_err = rd_kafka_header_get_last(rkm_headers, "REPLY_PARTITION", &header_value, &header_value_size))) {
                    log_print(ERROR, "Failed to read message header: %s", rd_kafka_err2str(r_err));
                } else {
                    reply_partition = (int_32) ntohl((uint_32) *((const int_32 *) header_value));

                    r_err = rd_kafka_producev(rk_p,
                                              RD_KAFKA_VTYPE_RKT, rkt_p,
                                              RD_KAFKA_VTYPE_PARTITION, reply_partition,
                                              RD_KAFKA_VTYPE_MSGFLAGS, RD_KAFKA_MSG_F_COPY,
                                              RD_KAFKA_VTYPE_VALUE, &reply, sizeof(reply),
                                              RD_KAFKA_VTYPE_END
                    );

                    if (RD_KAFKA_CONF_OK != r_err) {
                        log_print(ERROR, "Failed to delivery message to topic \"%s\", partition %d: %s", producer_topic, reply_partition, rd_kafka_err2str(r_err));
                    } else {
                        log_debug(NOISY, NOERR, "Enqueued message for topic \"%s\", partition %d", producer_topic, reply_partition);
                    }
                }
            }
        }

        rd_kafka_message_destroy(rkm);
    }
}
