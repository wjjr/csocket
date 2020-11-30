/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_INVOKER_SERVICE_H
#define CSOCKET_INVOKER_SERVICE_H

#include "m/data.h"

struct service;

struct service *service_new(const char *service_name, uint_8 methods_capacity);

void service_add_method(struct service *, const char *method, void (*func)(const data *request, data *reply));

void service_get_method(const struct service *, const char *method, void (**func)(const data *request, data *reply));

#endif /* CSOCKET_INVOKER_SERVICE_H */
