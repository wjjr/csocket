/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_REQUESTOR_H
#define CSOCKET_REQUESTOR_H

#include "np/types.h"
#include "m/data.h"
#include "rh/types.h"

struct requestor;

struct requestor *requestor_new(const struct host_addr *);

void requestor_destroy(struct requestor *);

bool requestor_is_active(struct requestor *);

bool requestor_invoke(struct requestor *, const char *method, const struct data *request, struct data **reply);

#endif /* CSOCKET_REQUESTOR_H */
