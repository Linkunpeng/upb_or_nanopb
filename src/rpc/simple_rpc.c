#include "simple_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_HANDLERS 16
#define MAX_PENDING_CONNECTIONS 5

// Server structure
struct rpc_server {
    int server_fd;
    uint16_t port;
    bool running;
    rpc_handler_t handlers[MAX_HANDLERS];
};

// Client structure
struct rpc_client {
    int socket_fd;
    char host[256];
    uint16_t port;
    bool connected;
    uint32_t next_request_id;
};

// Helper to send all data
static bool send_all(int socket_fd, const void* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t result = send(socket_fd, (const uint8_t*)data + sent, size - sent, 0);
        if (result < 0) {
            perror("send");
            return false;
        }
        sent += result;
    }
    return true;
}

// Helper to receive all data
static bool recv_all(int socket_fd, void* data, size_t size) {
    size_t received = 0;
    while (received < size) {
        ssize_t result = recv(socket_fd, (uint8_t*)data + received, size - received, 0);
        if (result < 0) {
            perror("recv");
            return false;
        }
        if (result == 0) {
            // Connection closed
            return false;
        }
        received += result;
    }
    return true;
}

// Server functions
rpc_server_t* rpc_server_create(uint16_t port) {
    rpc_server_t* server = (rpc_server_t*)calloc(1, sizeof(rpc_server_t));
    if (!server) {
        return NULL;
    }
    
    server->port = port;
    server->server_fd = -1;
    server->running = false;
    memset(server->handlers, 0, sizeof(server->handlers));
    
    return server;
}

void rpc_server_destroy(rpc_server_t* server) {
    if (!server) return;
    
    if (server->running) {
        rpc_server_stop(server);
    }
    
    if (server->server_fd >= 0) {
        close(server->server_fd);
    }
    
    free(server);
}

bool rpc_server_register_handler(rpc_server_t* server, rpc_method_t method, rpc_handler_t handler) {
    if (!server || method <= 0 || method >= MAX_HANDLERS) {
        return false;
    }
    
    server->handlers[method] = handler;
    return true;
}

bool rpc_server_start(rpc_server_t* server) {
    if (!server || server->running) {
        return false;
    }
    
    // Create socket
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        perror("socket");
        return false;
    }
    
    // Set SO_REUSEADDR to quickly restart server
    int opt = 1;
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server->server_fd);
        server->server_fd = -1;
        return false;
    }
    
    // Bind to port
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server->port);
    
    if (bind(server->server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server->server_fd);
        server->server_fd = -1;
        return false;
    }
    
    // Listen
    if (listen(server->server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen");
        close(server->server_fd);
        server->server_fd = -1;
        return false;
    }
    
    server->running = true;
    printf("RPC Server listening on port %d\n", server->port);
    return true;
}

void rpc_server_stop(rpc_server_t* server) {
    if (!server) return;
    
    server->running = false;
    if (server->server_fd >= 0) {
        close(server->server_fd);
        server->server_fd = -1;
    }
}

bool rpc_server_handle_request(rpc_server_t* server) {
    if (!server || !server->running) {
        return false;
    }
    
    // Accept connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("accept");
        }
        return false;
    }
    
    // Receive header
    rpc_header_t header;
    if (!recv_all(client_fd, &header, sizeof(header))) {
        close(client_fd);
        return false;
    }
    
    // Convert from network byte order
    header.method_id = ntohl(header.method_id);
    header.payload_size = ntohl(header.payload_size);
    header.request_id = ntohl(header.request_id);
    
    // Validate
    if (header.method_id <= 0 || header.method_id >= MAX_HANDLERS || 
        header.payload_size > 1024 * 1024) {  // 1MB max
        close(client_fd);
        return false;
    }
    
    // Get handler
    rpc_handler_t handler = server->handlers[header.method_id];
    if (!handler) {
        close(client_fd);
        return false;
    }
    
    // Receive request payload
    uint8_t* request_buf = (uint8_t*)malloc(header.payload_size);
    if (!request_buf) {
        close(client_fd);
        return false;
    }
    
    if (!recv_all(client_fd, request_buf, header.payload_size)) {
        free(request_buf);
        close(client_fd);
        return false;
    }
    
    // Call handler
    uint8_t response_buf[4096];
    size_t response_size = 0;
    bool success = handler(request_buf, header.payload_size,
                          response_buf, &response_size, sizeof(response_buf));
    
    free(request_buf);
    
    if (!success) {
        close(client_fd);
        return false;
    }
    
    // Send response header
    rpc_header_t response_header;
    response_header.method_id = htonl(header.method_id);
    response_header.payload_size = htonl((uint32_t)response_size);
    response_header.request_id = htonl(header.request_id);
    
    if (!send_all(client_fd, &response_header, sizeof(response_header))) {
        close(client_fd);
        return false;
    }
    
    // Send response payload
    if (response_size > 0) {
        if (!send_all(client_fd, response_buf, response_size)) {
            close(client_fd);
            return false;
        }
    }
    
    close(client_fd);
    return true;
}

// Client functions
rpc_client_t* rpc_client_create(const char* host, uint16_t port) {
    if (!host) return NULL;
    
    rpc_client_t* client = (rpc_client_t*)calloc(1, sizeof(rpc_client_t));
    if (!client) {
        return NULL;
    }
    
    strncpy(client->host, host, sizeof(client->host) - 1);
    client->port = port;
    client->socket_fd = -1;
    client->connected = false;
    client->next_request_id = 1;
    
    return client;
}

void rpc_client_destroy(rpc_client_t* client) {
    if (!client) return;
    
    if (client->connected) {
        rpc_client_disconnect(client);
    }
    
    free(client);
}

bool rpc_client_connect(rpc_client_t* client) {
    if (!client || client->connected) {
        return false;
    }
    
    // Create socket
    client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        perror("socket");
        return false;
    }
    
    // Connect to server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->port);
    
    if (inet_pton(AF_INET, client->host, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client->socket_fd);
        client->socket_fd = -1;
        return false;
    }
    
    if (connect(client->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client->socket_fd);
        client->socket_fd = -1;
        return false;
    }
    
    client->connected = true;
    return true;
}

void rpc_client_disconnect(rpc_client_t* client) {
    if (!client) return;
    
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
        client->socket_fd = -1;
    }
    
    client->connected = false;
}

bool rpc_client_call(rpc_client_t* client, rpc_method_t method,
                     const uint8_t* request, size_t request_size,
                     uint8_t* response, size_t* response_size, size_t response_max) {
    if (!client || !request || !response || !response_size) {
        return false;
    }
    
    // Connect if not connected
    if (!client->connected) {
        if (!rpc_client_connect(client)) {
            return false;
        }
    }
    
    // Send request header
    rpc_header_t header;
    header.method_id = htonl((uint32_t)method);
    header.payload_size = htonl((uint32_t)request_size);
    header.request_id = htonl(client->next_request_id++);
    
    if (!send_all(client->socket_fd, &header, sizeof(header))) {
        rpc_client_disconnect(client);
        return false;
    }
    
    // Send request payload
    if (request_size > 0) {
        if (!send_all(client->socket_fd, request, request_size)) {
            rpc_client_disconnect(client);
            return false;
        }
    }
    
    // Receive response header
    rpc_header_t response_header;
    if (!recv_all(client->socket_fd, &response_header, sizeof(response_header))) {
        rpc_client_disconnect(client);
        return false;
    }
    
    // Convert from network byte order
    response_header.method_id = ntohl(response_header.method_id);
    response_header.payload_size = ntohl(response_header.payload_size);
    response_header.request_id = ntohl(response_header.request_id);
    
    // Validate
    if (response_header.payload_size > response_max) {
        rpc_client_disconnect(client);
        return false;
    }
    
    // Receive response payload
    if (response_header.payload_size > 0) {
        if (!recv_all(client->socket_fd, response, response_header.payload_size)) {
            rpc_client_disconnect(client);
            return false;
        }
    }
    
    *response_size = response_header.payload_size;
    
    // Disconnect after each call (stateless RPC)
    rpc_client_disconnect(client);
    
    return true;
}
