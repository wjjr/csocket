/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "log.h"
#include "common.h"
#include "kafka.h"

static rd_kafka_topic_partition_list_t *current_partitions = NULL;

static void rebalance_callback(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *partitions, void *_ __UNUSED) {
    int i;
    rd_kafka_topic_partition_list_t *assigned_partitions;

    switch (err) {
        case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
            for (i = 0; i < partitions->cnt; ++i) {
                partitions->elems[i].offset = RD_KAFKA_OFFSET_END;

                log_debug(INFO, NOERR, "Assign partition: Topic=\"%s\", Partition=%d, Offset=%ld",
                          partitions->elems[i].topic, partitions->elems[i].partition, partitions->elems[i].offset);
            }

            assigned_partitions = rd_kafka_topic_partition_list_copy(partitions);
            rd_kafka_assign(rk, partitions);
            current_partitions = assigned_partitions;
            break;
        case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
            current_partitions = NULL;

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

void run_client(const struct context *const ctx __UNUSED) {
    const char *broker_id = "127.0.0.1:9092",
            *group_id = "server",
            *consumer_topic = "reply",
            *producer_topic = "request";
    rd_kafka_conf_t *rkc = rd_kafka_conf_new();
    rd_kafka_t *rk_c, *rk_p;
    rd_kafka_topic_t *rkt_p;
    rd_kafka_topic_partition_list_t *subscription_list = rd_kafka_topic_partition_list_new(1);
    rd_kafka_resp_err_t r_err;
    rd_kafka_message_t *rkm = NULL;
    int_32 reply_partition = RD_KAFKA_PARTITION_UA;
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
        if (NULL == current_partitions || 0 >= current_partitions->cnt) {
            log_print(INFO, "Waiting for rebalance...");
            rd_kafka_consumer_poll(rk_c, 2000);
            continue;
        } else
            reply_partition = (int_32) htonl((uint_32) current_partitions->elems[current_partitions->cnt - 1].partition); /* NOLINT(cert-msc50-cpp) */

        request.a = htons((rand() % UINT16_MAX) + 1); /* NOLINT(cert-msc30-c,cert-msc50-cpp) */
        request.b = htons((rand() % UINT16_MAX) + 1); /* NOLINT(cert-msc30-c,cert-msc50-cpp) */
        request.op = (rand() % 2) == 0 ? ADD : SUB;   /* NOLINT(cert-msc30-c,cert-msc50-cpp) */

        r_err = rd_kafka_producev(rk_p,
                                  RD_KAFKA_VTYPE_RKT, rkt_p,
                                  RD_KAFKA_VTYPE_PARTITION, RD_KAFKA_PARTITION_UA,
                                  RD_KAFKA_VTYPE_MSGFLAGS, RD_KAFKA_MSG_F_COPY,
                                  RD_KAFKA_VTYPE_VALUE, &request, sizeof(request),
                                  RD_KAFKA_VTYPE_HEADER, "REPLY_PARTITION", &reply_partition, sizeof(int_32),
                                  RD_KAFKA_VTYPE_END
        );

        if (RD_KAFKA_CONF_OK != r_err) {
            log_print(WARN, "Failed to delivery message to topic \"%s\": %s", producer_topic, rd_kafka_err2str(r_err));
        } else {
            log_debug(NOISY, NOERR, "Enqueued message for topic \"%s\". Waiting for \"%s\" on partition %d", producer_topic, consumer_topic, (int_32) ntohl((uint_32) reply_partition));
        }

        if (NULL == (rkm = rd_kafka_consumer_poll(rk_c, 5000))) {
            log_print(WARN, "No \"%s\" to consume", consumer_topic);
            continue;
        }

        if (RD_KAFKA_RESP_ERR_NO_ERROR != rkm->err)
            log_print(WARN, "Failed to consume from \"%s\": %s", consumer_topic, rd_kafka_message_errstr(rkm));
        else
            log_debug(NOISY, NOERR, "Message %"PRId64" received from topic \"%s\", partition %d", rkm->offset, rd_kafka_topic_name(rkm->rkt), rkm->partition);

        if (NULL != rkm->payload && sizeof(int_32) == rkm->len) {
            reply = *((int_32 *) rkm->payload);

            if (reply == calc(request))
                log_debug(NOISY, NOERR, "Reply: %d %c %d = %d", ntohs(request.a), request.op, ntohs(request.b), (int_32) ntohl((uint_32) reply));
            else
                log_debug(ERROR, NOERR, "Reply: %d %c %d != %d", ntohs(request.a), request.op, ntohs(request.b), (int_32) ntohl((uint_32) reply));
        }

        rd_kafka_message_destroy(rkm);
    }
}

uint_8 run_client_benchmark(const struct context *const ctx) {
    double total_time = 0, *times = malloc(sizeof(double) * ctx->benchmark_num), min, avg, max = 0, mdev = 0;
    clock_t begin;

    for (uint_16 i = 0; i < ctx->benchmark_num; ++i) {
        begin = clock();
        /* TODO: SEND_REQUEST */
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
