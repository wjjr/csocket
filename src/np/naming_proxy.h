/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_NAMING_PROXY_H
#define CSOCKET_NAMING_PROXY_H

#include "types/primitive.h"
#include "types.h"

bool np_lookup(const char *service_name, const struct host_addr **host_addr);

bool np_add_service(const char *service_name, const char *hostname);

#endif /* CSOCKET_NAMING_PROXY_H */
