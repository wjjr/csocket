/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "server.h"

#include "i/invoker.h"
#include "i/service.h"

static bool data_to_params(const data *d, uint_16 *const a, uint_16 *const b) {
    const value *const v_a = data_get_value(d, 0);
    const value *const v_b = data_get_value(d, 1);

    if (v_a != NULL && v_a->type == UINT && v_a->size == 2 && v_b != NULL && v_b->type == UINT && v_b->size == 2) {
        *a = *((uint_16 *) v_a->value);
        *b = *((uint_16 *) v_b->value);
        return true;
    }

    return false;
}

static void calc_add(const data *const d, data *const r) {
    uint_16 a, b;
    int_32 result;

    if (data_to_params(d, &a, &b)) {
        result = a + b;
        data_push(r, INT, sizeof(int_32), &result);
    }
}

static void calc_sub(const data *const d, data *const r) {
    uint_16 a, b;
    int_32 result;

    if (data_to_params(d, &a, &b)) {
        result = a - b;
        data_push(r, INT, sizeof(int_32), &result);
    }
}

static void calc_mul(const data *const d, data *const r) {
    uint_16 a, b;
    int_32 result;

    if (data_to_params(d, &a, &b)) {
        result = a * b;
        data_push(r, INT, sizeof(int_32), &result);
    }
}

static void calc_div(const data *const d, data *const r) {
    uint_16 a, b;
    int_32 result;

    if (data_to_params(d, &a, &b)) {
        result = a / b;
        data_push(r, INT, sizeof(int_32), &result);
    }
}

void run_server(const enum protocol protocol, const uint_16 port, const uint_8 thread_num) {
    struct service *service = service_new("calc", 4);
    struct invoker *invoker = invoker_new(protocol, port, thread_num);

    service_add_method(service, "add", calc_add);
    service_add_method(service, "sub", calc_sub);
    service_add_method(service, "mul", calc_mul);
    service_add_method(service, "div", calc_div);

    invoker_run(invoker, service);
}
