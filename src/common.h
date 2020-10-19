/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_COMMON_H
#define _CSOCKET_COMMON_H

#include <librdkafka/rdkafka.h>

static int_32 calc(const struct request r) {
    switch (r.op) {
        case ADD:
            return (int_32) htonl(ntohs(r.a) + ntohs(r.b));
        case SUB:
            return (int_32) htonl(ntohs(r.a) - ntohs(r.b));
        case MUL:
            return (int_32) htonl(ntohs(r.a) * ntohs(r.b));
        case DIV:
            return (int_32) htonl(ntohs(r.a) / ntohs(r.b));
    }

    log_debug(ERROR, NOERR, "Wrong operation");

    return (int_32) htonl((uint_32) INT32_MIN);
}

static void rebalance_callback(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *partitions, void *rebalanced) {
    int i;

    switch (err) {
        case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
            for (i = 0; i < partitions->cnt; ++i) {
                partitions->elems[i].offset = RD_KAFKA_OFFSET_END;

                log_print(NOISY, "Assign partition: Topic=\"%s\", Partition=%d, Offset=%ld",
                          partitions->elems[i].topic, partitions->elems[i].partition, partitions->elems[i].offset);
            }

            rd_kafka_assign(rk, partitions);

            if (rebalanced) *((bool *) rebalanced) = true;
            break;
        case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
            if (rebalanced) *((bool *) rebalanced) = false;

            for (i = 0; i < partitions->cnt; ++i) {
                log_print(NOISY, "Revoke partition: Topic=\"%s\", Partition=%d, Offset=%ld",
                          partitions->elems[i].topic, partitions->elems[i].partition, partitions->elems[i].offset);
            }

            rd_kafka_assign(rk, NULL);
            break;
        default:
            rd_kafka_assign(rk, NULL);
    }

    log_print(INFO, "Rebalanced");
}

#endif /* _CSOCKET_COMMON_H */
