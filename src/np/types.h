/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_NAMING_PROXY_TYPES_H
#define CSOCKET_NAMING_PROXY_TYPES_H

#include "types/primitive.h"
#include "rh/types.h"

struct host_addr {
    char *service_name;
    enum protocol protocol;
    char *address;
    uint_16 port;
};

#endif /* CSOCKET_NAMING_PROXY_TYPES_H */
