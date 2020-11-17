/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_RH_CLIENT_H
#define CSOCKET_RH_CLIENT_H

#include "types/primitive.h"
#include "types.h"

typedef struct rh_conn_ctx rh_conn_ctx;

typedef struct {
    byte *data;
    usize data_size;
} rh_server_msg;

rh_conn_ctx *rh_client_new(enum protocol, const char *host, uint_16 port);

bool rh_send_to_server(rh_conn_ctx *, const byte *data, usize data_size);

rh_server_msg *rh_receive_from_server(rh_conn_ctx *);

void rh_server_msg_destroy(rh_server_msg *server_msg);

void rh_client_destroy(rh_conn_ctx *);

#endif /* CSOCKET_RH_CLIENT_H */
