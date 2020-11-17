/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_MARSHALLER_DATA_H
#define CSOCKET_MARSHALLER_DATA_H

#include "types/primitive.h"

enum type {
    BYTES,
    INT,
    UINT
};

typedef struct value {
    enum type type;
    usize size;
    void *value;
} value;

typedef struct data data;

struct data *data_new(uint8_t size);

void data_destroy(struct data *);

void data_push(struct data *, enum type type, uint8_t size, const void *value);

void data_pop(struct data *, struct value *);

uint_8 data_size(const struct data*);

const struct value *data_get_value(const struct data *, uint_8 index);

#endif /* CSOCKET_MARSHALLER_DATA_H */
