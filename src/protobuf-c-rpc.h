#ifndef _CSOCKET_PROTOBUF_C_RPC_H
#define _CSOCKET_PROTOBUF_C_RPC_H

#include <protobuf-c-rpc/protobuf-c-rpc.h>
#include "pb/calc.pb-c.h"
#include "log.h"

typedef struct _ProtobufC_RPC_Server Server;
typedef struct _ProtobufC_RPC_Client Client;
typedef struct ProtobufCService Service;
typedef struct _ProtobufCRPCDispatch Dispatch;

static void error_handler(ProtobufC_RPC_Error_Code code, const char *message, void *error_func_data __attribute__((unused))) {
    char *error_code = "UNKNOWN";

    switch (code) {
        case PROTOBUF_C_RPC_ERROR_CODE_HOST_NOT_FOUND:
            error_code = "HOST_NOT_FOUND";
            break;
        case PROTOBUF_C_RPC_ERROR_CODE_CONNECTION_REFUSED:
            error_code = "CONNECTION_REFUSED";
            break;
        case PROTOBUF_C_RPC_ERROR_CODE_CLIENT_TERMINATED:
            error_code = "CLIENT_TERMINATED";
            break;
        case PROTOBUF_C_RPC_ERROR_CODE_BAD_REQUEST:
            error_code = "CODE_BAD_REQUEST";
            break;
        case PROTOBUF_C_RPC_ERROR_CODE_PROXY_PROBLEM:
            error_code = "CODE_PROXY_PROBLEM";
            break;
    }

    log_print(ERROR, "%s: %s", error_code, message);
}

static __always_inline Server *new_server(const char *name, Service *service) {
    Server *server = protobuf_c_rpc_server_new(PROTOBUF_C_RPC_ADDRESS_TCP, name, service, protobuf_c_rpc_dispatch_default());

    if (server)
        protobuf_c_rpc_server_set_error_handler(server, error_handler, NULL);

    return server;
}

static void idle(Dispatch *dispatch __attribute__((unused)), void *func_data __attribute__((unused))) {
}

static __always_inline void dispatch_run(void) {
    protobuf_c_rpc_dispatch_add_idle(protobuf_c_rpc_dispatch_default(), idle, NULL);
    protobuf_c_rpc_dispatch_run(protobuf_c_rpc_dispatch_default());
}

static __always_inline Service *new_service(const char *const hostname) {
    Service *service = protobuf_c_rpc_client_new(PROTOBUF_C_RPC_ADDRESS_TCP, hostname, &calc__descriptor, protobuf_c_rpc_dispatch_default());
    protobuf_c_rpc_client_set_error_handler((Client *) service, error_handler, NULL);

    return service;
}

static __always_inline bool service_is_connected(Service *service) {
    return (bool) protobuf_c_rpc_client_is_connected((Client *) service);
}

static __always_inline void service_destroy(Service *service) {
    protobuf_c_service_destroy(service);
}

#endif //_CSOCKET_PROTOBUF_C_RPC_H
