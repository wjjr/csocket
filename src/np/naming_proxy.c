/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "naming_proxy.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

static uint_8 services_count = 0;
static struct host_addr *services = NULL;

bool np_lookup(const char *service_name, const struct host_addr **const host_addr) {
    for (uint_8 i = 0; i < services_count; ++i) {
        if (strcmp(service_name, services[i].service_name) == 0) {
            *host_addr = &services[i];
            return true;
        }
    }

    *host_addr = NULL;
    return false;
}

bool np_add_service(const char *service_name __attribute__((unused)), const char *hostname __attribute__((unused))) {
    char service_address[256], *endptr;
    const char *proto, *address, *port;
    long port_number;

    if (services == NULL)
        services = malloc(sizeof(struct host_addr) * 256);

    strcpy(service_address, hostname);

    proto = strtok(service_address, ":");
    address = strtok(NULL, "://");
    port = strtok(NULL, ":");

    port_number = strtol(port, &endptr, 10);
    if (*endptr != '\0' || port_number <= 0 || port_number > USHRT_MAX || endptr == port)
        return false;

    services[services_count].service_name = malloc(strlen(service_name) + 1);
    strcpy(services[services_count].service_name, service_name);
    services[services_count].address = malloc(strlen(address) + 1);
    strcpy(services[services_count].address, address);
    services[services_count].port = (uint_16) port_number;

    if (strcmp("tcp", proto) == 0)
        services[services_count].protocol = TCP;
    else
        services[services_count].protocol = UDP;

    ++services_count;

    return true;
}
