/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "service.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct method {
    const char *method;
    void (*func)(const struct data *, struct data *);
};

struct service {
    const char *name;
    uint_8 methods_capacity;
    uint_8 methods_count;
    struct method *methods;
    uint_8 instances_count;
    struct service_instance **instances;
    pthread_mutex_t instances_mutex;
    pthread_cond_t instances_cond;
};

struct service_instance {
    struct service *service;
};

struct service *service_new(const char *const service_name, const uint_8 methods_capacity, const uint_8 instances_num) {
    struct service *service = malloc(sizeof(struct service));

    service->name = service_name;
    service->methods_capacity = methods_capacity;
    service->methods_count = 0;
    service->methods = malloc(sizeof(struct method) * methods_capacity);
    service->instances_count = instances_num;
    service->instances = malloc(sizeof(struct service_instance *) * instances_num);
    pthread_mutex_init(&service->instances_mutex, NULL);
    pthread_cond_init(&service->instances_cond, NULL);

    for (uint_8 i = 0; i < instances_num; ++i) {
        service->instances[i] = malloc(sizeof(struct service_instance));
        service->instances[i]->service = service;
    }

    return service;
}

void service_add_method(struct service *const service, const char *const method_name, service_method *const func) {
    if (service->methods_count < service->methods_capacity) {
        service->methods[service->methods_count].method = method_name;
        service->methods[service->methods_count].func = func;
        ++service->methods_count;
    }
}

struct service_instance *service_get_instance(struct service *const service) {
    pthread_mutex_lock(&service->instances_mutex);

    while (service->instances_count == 0)
        pthread_cond_wait(&service->instances_cond, &service->instances_mutex);

    uint_8 i = --service->instances_count;
    struct service_instance *service_instance = service->instances[i];
    service->instances[i] = NULL;

    pthread_mutex_unlock(&service->instances_mutex);

    return service_instance;
}

service_method *service_get_method(const struct service_instance *const service_instance, const char *const method_name) {
    for (uint_8 i = 0; i < service_instance->service->methods_count; ++i) {
        if (strcmp(method_name, service_instance->service->methods[i].method) == 0) {
            return service_instance->service->methods[i].func;
        }
    }

    return NULL;
}

void service_release_instance(struct service *const service, struct service_instance *const service_instance) {
    pthread_mutex_lock(&service->instances_mutex);

    service->instances[service->instances_count++] = service_instance;

    pthread_cond_signal(&service->instances_cond);
    pthread_mutex_unlock(&service->instances_mutex);
}
