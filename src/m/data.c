/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "data.h"

#include <stdlib.h>
#include <memory.h>

struct data {
    uint8_t capacity;
    uint8_t count;
    struct value *values;
};

struct data *data_new(const uint8_t size) {
    struct data *data = malloc(sizeof(struct data));

    data->count = 0;
    data->capacity = size;
    data->values = calloc(size + 1, sizeof(struct value));

    return data;
}

void data_destroy(struct data *const data) {
    uint_8 i;

    for (i = 0; data->values[i].value != NULL; ++i)
        free(data->values[i].value);

    free(data->values);
    free(data);
}

static void data_resize(struct data *const data, const uint8_t size) {
    data->capacity = size;
    data->values = realloc(data->values, (sizeof(struct value) * (size + 1)));
    data->values[size].value = NULL;
}

void data_push(struct data *const data, const enum type type, const uint8_t size, const void *const value) {
    struct value *data_value;

    if (data->count == data->capacity)
        data_resize(data, data->capacity + 2);

    data_value = &data->values[data->count++];

    data_value->type = type;
    data_value->size = size;
    data_value->value = malloc(size);
    memcpy(data_value->value, value, size);
}

void data_pop(struct data *const data, struct value *const value) {
    if (data->count > 0)
        *value = data->values[--data->count];
    else
        (*value).value = NULL;
}

uint_8 data_size(const struct data *const data) {
    return data->count;
}

const struct value *data_get_value(const struct data *const data, const uint_8 index) {
    if (data->count > index)
        return &data->values[index];

    return NULL;
}
