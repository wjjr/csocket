/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "client.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <uuid/uuid.h>
#include "log.h"
#include "common.h"
#include "kafka.h"

static bool rebalanced = false;

static const char *uuid_gen() {
    uuid_t binary_uuid;
    char *uuid = malloc(37);

    uuid_generate(binary_uuid);
    uuid_unparse_lower(binary_uuid, uuid);

    return uuid;
}

void run_client(const struct context *const ctx __UNUSED) {
    const char *broker_id = "127.0.0.1:9092",
            *client_id = uuid_gen(),
            *group_id = uuid_gen(),
            *producer_topic = "request";
    size_t client_id_len = strlen(client_id);
    rd_kafka_conf_t *rkc = rd_kafka_conf_new();
    rd_kafka_t *rk_c, *rk_p;
    rd_kafka_topic_t *rkt_c;
    rd_kafka_resp_err_t r_err;
    rd_kafka_message_t *rkm = NULL;
    char err_str[512];
    struct request request;
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

    if (NULL == (rkt_c = rd_kafka_topic_new(rk_p, client_id, NULL)))
        die(EXIT_FAILURE, NOERR, "Failed to create new topic: %s", rd_kafka_err2str(rd_kafka_last_error()));
    else {
        if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = rd_kafka_produce(rkt_c, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, "0123456789", 10, NULL, 0, NULL)))
            die(EXIT_FAILURE, NOERR, "Failed to delivery message to topic \"%s\": %s", client_id, rd_kafka_err2str(r_err));

        rd_kafka_topic_destroy(rkt_c);
        sleep(1);
    }

    if (RD_KAFKA_RESP_ERR_NO_ERROR != (r_err = kafka_subscribe(rk_c, client_id, RD_KAFKA_PARTITION_UA)))
        die(EXIT_FAILURE, NOERR, "Failed to subscribe to \"%s\": %s", client_id, rd_kafka_err2str(r_err));

    while (!rebalanced)
        kafka_consumer_poll(rk_c, 200);

    log_print(INFO, "Started");

    for (;;) {
        request.a = htons((rand() % UINT16_MAX) + 1); /* NOLINT(cert-msc30-c,cert-msc50-cpp) */
        request.b = htons((rand() % UINT16_MAX) + 1); /* NOLINT(cert-msc30-c,cert-msc50-cpp) */
        request.op = (rand() % 2) == 0 ? ADD : SUB;   /* NOLINT(cert-msc30-c,cert-msc50-cpp) */

        r_err = rd_kafka_producev(rk_p,
                                  RD_KAFKA_VTYPE_TOPIC, producer_topic,
                                  RD_KAFKA_VTYPE_PARTITION, RD_KAFKA_PARTITION_UA,
                                  RD_KAFKA_VTYPE_MSGFLAGS, RD_KAFKA_MSG_F_COPY,
                                  RD_KAFKA_VTYPE_VALUE, &request, sizeof(request),
                                  RD_KAFKA_VTYPE_HEADER, "REPLY_TOPIC", client_id, client_id_len,
                                  RD_KAFKA_VTYPE_END
        );

        if (RD_KAFKA_RESP_ERR_NO_ERROR != r_err) {
            log_print(WARN, "Failed to delivery message to topic \"%s\": %s", producer_topic, rd_kafka_err2str(r_err));
        } else {
            log_debug(NOISY, NOERR, "Enqueued message for topic \"%s\"", producer_topic);
        }

        if (NULL == (rkm = kafka_consumer_poll(rk_c, 5000))) {
            log_print(WARN, "No reply");
            continue;
        }

        if (RD_KAFKA_RESP_ERR_NO_ERROR != rkm->err)
            log_print(WARN, "Failed to consume from \"%s\": %s", client_id, rd_kafka_message_errstr(rkm));
        else {
            log_print(NOISY, "Message %"PRId64" received from topic \"%s\"", rkm->offset, rd_kafka_topic_name(rkm->rkt));

            if (NULL != rkm->payload && sizeof(int_32) == rkm->len) {
                reply = *((int_32 *) rkm->payload);

                if (reply == calc(request))
                    log_debug(NOISY, NOERR, "Reply: %d %c %d = %d", ntohs(request.a), request.op, ntohs(request.b), (int_32) ntohl((uint_32) reply));
                else
                    log_debug(ERROR, NOERR, "Reply: %d %c %d != %d", ntohs(request.a), request.op, ntohs(request.b), (int_32) ntohl((uint_32) reply));
            }
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
