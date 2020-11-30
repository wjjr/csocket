/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "service.h"

#include <stdlib.h>
#include <string.h>

struct method {
    const char *method;
    void (*func)(const struct data *, struct data *);
};

struct service {
    const char *name;
    uint_8 methods_capacity;
    uint_8 methods_count;
    struct method *methods;
};

struct service *service_new(const char *const service_name, const uint_8 methods_capacity) {
    struct service *service = malloc(sizeof(struct service));

    service->name = service_name;
    service->methods_capacity = methods_capacity;
    service->methods_count = 0;
    service->methods = malloc(sizeof(struct method) * methods_capacity);

    return service;
}

void service_add_method(struct service *const service, const char *const method, void (*const func)(const struct data *, struct data *)) {
    if (service->methods_count < service->methods_capacity) {
        service->methods[service->methods_count].method = method;
        service->methods[service->methods_count].func = func;
        ++service->methods_count;
    }
}

void service_get_method(const struct service *service, const char *method, void (**const func)(const data *, data *)) {
    for (uint_8 i = 0; i < service->methods_count; ++i) {
        if (strcmp(method, service->methods[i].method) == 0) {
            *func = service->methods[i].func;
            return;
        }
    }

    *func = NULL;
}
