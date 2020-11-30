/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_INVOKER_SERVICE_H
#define CSOCKET_INVOKER_SERVICE_H

#include "m/data.h"

typedef void (service_method)(const data *, data *);

struct service;

struct service_instance;

struct service *service_new(const char *service_name, uint_8 methods_capacity, uint_8 instances_num);

void service_add_method(struct service *, const char *method_name, service_method *);

struct service_instance *service_get_instance(struct service *);

service_method *service_get_method(const struct service_instance *, const char *method_name);

void service_release_instance(struct service *service, struct service_instance *);

#endif /* CSOCKET_INVOKER_SERVICE_H */
