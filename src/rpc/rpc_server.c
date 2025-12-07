#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "simple_rpc.h"
#include "person.pb.h"
#include "person_service.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

// Simple in-memory storage for persons
#define MAX_PERSONS 100
static demo_Person persons[MAX_PERSONS];
static int person_count = 0;

// Global server pointer for signal handling
static rpc_server_t* g_server = NULL;

// Signal handler for graceful shutdown
static void signal_handler(int signum) {
    (void)signum;
    printf("\nShutting down server...\n");
    if (g_server) {
        rpc_server_stop(g_server);
    }
}

// Handler for GetPerson RPC
static bool handle_get_person(const uint8_t* request, size_t request_size,
                               uint8_t* response, size_t* response_size,
                               size_t response_max) {
    // Decode request
    demo_GetPersonRequest req = demo_GetPersonRequest_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(request, request_size);
    
    if (!pb_decode(&istream, demo_GetPersonRequest_fields, &req)) {
        fprintf(stderr, "Failed to decode GetPersonRequest\n");
        return false;
    }
    
    printf("GetPerson: id=%d\n", req.id);
    
    // Search for person
    demo_GetPersonResponse resp = demo_GetPersonResponse_init_zero;
    resp.found = false;
    
    for (int i = 0; i < person_count; i++) {
        if (persons[i].id == req.id) {
            resp.person = persons[i];
            resp.found = true;
            break;
        }
    }
    
    // Encode response
    pb_ostream_t ostream = pb_ostream_from_buffer(response, response_max);
    if (!pb_encode(&ostream, demo_GetPersonResponse_fields, &resp)) {
        fprintf(stderr, "Failed to encode GetPersonResponse\n");
        return false;
    }
    
    *response_size = ostream.bytes_written;
    printf("  -> Found: %s\n", resp.found ? "yes" : "no");
    return true;
}

// Handler for AddPerson RPC
static bool handle_add_person(const uint8_t* request, size_t request_size,
                               uint8_t* response, size_t* response_size,
                               size_t response_max) {
    // Decode request
    demo_AddPersonRequest req = demo_AddPersonRequest_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(request, request_size);
    
    if (!pb_decode(&istream, demo_AddPersonRequest_fields, &req)) {
        fprintf(stderr, "Failed to decode AddPersonRequest\n");
        return false;
    }
    
    printf("AddPerson: name=%s, id=%d\n", req.person.name, req.person.id);
    
    // Prepare response
    demo_AddPersonResponse resp = demo_AddPersonResponse_init_zero;
    
    // Check if person already exists
    bool exists = false;
    for (int i = 0; i < person_count; i++) {
        if (persons[i].id == req.person.id) {
            exists = true;
            break;
        }
    }
    
    if (exists) {
        resp.success = false;
        strncpy(resp.message, "Person with this ID already exists", sizeof(resp.message) - 1);
    } else if (person_count >= MAX_PERSONS) {
        resp.success = false;
        strncpy(resp.message, "Storage full", sizeof(resp.message) - 1);
    } else {
        // Add person
        persons[person_count++] = req.person;
        resp.success = true;
        strncpy(resp.message, "Person added successfully", sizeof(resp.message) - 1);
    }
    
    // Encode response
    pb_ostream_t ostream = pb_ostream_from_buffer(response, response_max);
    if (!pb_encode(&ostream, demo_AddPersonResponse_fields, &resp)) {
        fprintf(stderr, "Failed to encode AddPersonResponse\n");
        return false;
    }
    
    *response_size = ostream.bytes_written;
    printf("  -> Success: %s, message: %s\n", resp.success ? "yes" : "no", resp.message);
    return true;
}

// Handler for ListPersons RPC
static bool handle_list_persons(const uint8_t* request, size_t request_size,
                                 uint8_t* response, size_t* response_size,
                                 size_t response_max) {
    // Decode request
    demo_ListPersonsRequest req = demo_ListPersonsRequest_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(request, request_size);
    
    if (!pb_decode(&istream, demo_ListPersonsRequest_fields, &req)) {
        fprintf(stderr, "Failed to decode ListPersonsRequest\n");
        return false;
    }
    
    printf("ListPersons: max_results=%d\n", req.max_results);
    
    // Prepare response
    demo_ListPersonsResponse resp = demo_ListPersonsResponse_init_zero;
    resp.total_count = person_count;
    
    // Determine how many to return
    int max_results = req.max_results > 0 ? req.max_results : person_count;
    if (max_results > person_count) {
        max_results = person_count;
    }
    if (max_results > 10) {  // Limited by max_count in .options file
        max_results = 10;
    }
    
    // Copy persons
    resp.persons_count = max_results;
    for (int i = 0; i < max_results; i++) {
        resp.persons[i] = persons[i];
    }
    
    // Encode response
    pb_ostream_t ostream = pb_ostream_from_buffer(response, response_max);
    if (!pb_encode(&ostream, demo_ListPersonsResponse_fields, &resp)) {
        fprintf(stderr, "Failed to encode ListPersonsResponse\n");
        return false;
    }
    
    *response_size = ostream.bytes_written;
    printf("  -> Returning %d persons (total: %d)\n", max_results, person_count);
    return true;
}

int main(int argc, char* argv[]) {
    uint16_t port = 8080;
    
    if (argc > 1) {
        port = (uint16_t)atoi(argv[1]);
    }
    
    printf("Starting RPC Server on port %d\n", port);
    printf("Ctrl+C to stop\n\n");
    
    // Create server
    g_server = rpc_server_create(port);
    if (!g_server) {
        fprintf(stderr, "Failed to create server\n");
        return 1;
    }
    
    // Register signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Register handlers
    rpc_server_register_handler(g_server, RPC_METHOD_GET_PERSON, handle_get_person);
    rpc_server_register_handler(g_server, RPC_METHOD_ADD_PERSON, handle_add_person);
    rpc_server_register_handler(g_server, RPC_METHOD_LIST_PERSONS, handle_list_persons);
    
    // Start server
    if (!rpc_server_start(g_server)) {
        fprintf(stderr, "Failed to start server\n");
        rpc_server_destroy(g_server);
        return 1;
    }
    
    // Add some sample data
    demo_Person sample = demo_Person_init_zero;
    sample.id = 1;
    strncpy(sample.name, "Alice", sizeof(sample.name) - 1);
    strncpy(sample.email, "alice@example.com", sizeof(sample.email) - 1);
    sample.phones_count = 1;
    strncpy(sample.phones[0].number, "555-1111", sizeof(sample.phones[0].number) - 1);
    sample.phones[0].type = demo_Person_PhoneType_MOBILE;
    persons[person_count++] = sample;
    
    sample.id = 2;
    strncpy(sample.name, "Bob", sizeof(sample.name) - 1);
    strncpy(sample.email, "bob@example.com", sizeof(sample.email) - 1);
    sample.phones_count = 1;
    strncpy(sample.phones[0].number, "555-2222", sizeof(sample.phones[0].number) - 1);
    sample.phones[0].type = demo_Person_PhoneType_WORK;
    persons[person_count++] = sample;
    
    printf("Added %d sample persons\n\n", person_count);
    
    // Main server loop
    while (g_server && rpc_server_handle_request(g_server)) {
        // Continue processing requests
    }
    
    printf("Server stopped\n");
    rpc_server_destroy(g_server);
    g_server = NULL;
    
    return 0;
}
