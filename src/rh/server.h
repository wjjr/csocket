/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef CSOCKET_RH_SERVER_H
#define CSOCKET_RH_SERVER_H

#include "../types.h"

typedef struct rh_server_ctx rh_server_ctx;
typedef struct rh_client_addr rh_client_addr;

typedef struct {
    byte *data;
    ssize data_size;
    rh_client_addr *return_addr;
} rh_client_msg;

rh_server_ctx *rh_server_new(enum protocol, uint_16 port_to_listen);

rh_client_msg *rh_receive_from_client(const rh_server_ctx *);

bool rh_send_to_client(const rh_client_addr *, const byte *data, ssize data_size);

void rh_client_msg_destroy(rh_client_msg *);

#endif /* CSOCKET_RH_SERVER_H */
