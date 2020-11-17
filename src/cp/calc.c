/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "calc.h"

#include <errno.h>
#include "np/naming_proxy.h"
#include "r/requestor.h"

static const struct host_addr *host_addr = NULL;

static struct requestor *requestor = NULL;

static bool calc_invoke(const char *method, const uint_16 a, const uint_16 b, int_32 *const result) {
    struct data *request, *reply = NULL;
    struct value reply_value;

    if (requestor != NULL && !requestor_is_active(requestor)) {
        requestor_destroy(requestor);
        requestor = NULL;
    }

    if (requestor == NULL && (host_addr != NULL || np_lookup("calc", &host_addr)))
        requestor = requestor_new(host_addr);

    if (requestor != NULL) {
        request = data_new(2);
        data_push(request, UINT, sizeof(uint_16), &a);
        data_push(request, UINT, sizeof(uint_16), &b);

        if (requestor_invoke(requestor, method, request, &reply)) {
            data_pop(reply, &reply_value);

            if (reply_value.value != NULL && reply_value.type == INT && reply_value.size == sizeof(int_32)) {
                *result = *((int_32 *) reply_value.value);

                data_destroy(request);
                data_destroy(reply);

                return true;
            } else
                errno = ENOMSG;

            data_destroy(reply);
        }

        data_destroy(request);
    } else
        errno = EHOSTUNREACH;

    return false;
}

bool calc_add(const uint_16 a, const uint_16 b, int_32 *const result) {
    return calc_invoke("add", a, b, result);
}

bool calc_sub(const uint_16 a, const uint_16 b, int_32 *const result) {
    return calc_invoke("sub", a, b, result);
}

bool calc_mul(const uint_16 a, const uint_16 b, int_32 *const result) {
    return calc_invoke("mul", a, b, result);
}

bool calc_div(const uint_16 a, const uint_16 b, int_32 *const result) {
    return calc_invoke("div", a, b, result);
}
