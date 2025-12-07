#ifndef SIMPLE_RPC_H
#define SIMPLE_RPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>  // for size_t

// RPC method IDs
typedef enum {
    RPC_METHOD_GET_PERSON = 1,
    RPC_METHOD_ADD_PERSON = 2,
    RPC_METHOD_LIST_PERSONS = 3
} rpc_method_t;

// RPC message header (fixed size for simplicity)
typedef struct {
    uint32_t method_id;      // Which RPC method to call
    uint32_t payload_size;   // Size of the protobuf payload
    uint32_t request_id;     // ID to match requests with responses
} rpc_header_t;

// RPC server context
typedef struct rpc_server rpc_server_t;

// RPC client handle
typedef struct rpc_client rpc_client_t;

// Handler function type for RPC methods
typedef bool (*rpc_handler_t)(const uint8_t* request, size_t request_size,
                               uint8_t* response, size_t* response_size,
                               size_t response_max);

// Server functions
rpc_server_t* rpc_server_create(uint16_t port);
void rpc_server_destroy(rpc_server_t* server);
bool rpc_server_register_handler(rpc_server_t* server, rpc_method_t method, rpc_handler_t handler);
bool rpc_server_start(rpc_server_t* server);
void rpc_server_stop(rpc_server_t* server);
bool rpc_server_handle_request(rpc_server_t* server); // Process one request

// Client functions
rpc_client_t* rpc_client_create(const char* host, uint16_t port);
void rpc_client_destroy(rpc_client_t* client);
bool rpc_client_connect(rpc_client_t* client);
void rpc_client_disconnect(rpc_client_t* client);
bool rpc_client_call(rpc_client_t* client, rpc_method_t method,
                     const uint8_t* request, size_t request_size,
                     uint8_t* response, size_t* response_size, size_t response_max);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_RPC_H
