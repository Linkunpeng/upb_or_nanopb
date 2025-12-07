#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_rpc.h"
#include "person.pb.h"
#include "person_service.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

// Call GetPerson RPC
static bool call_get_person(rpc_client_t* client, int32_t id) {
    printf("\n--- GetPerson(id=%d) ---\n", id);
    
    // Create request
    demo_GetPersonRequest req = demo_GetPersonRequest_init_zero;
    req.id = id;
    
    // Encode request
    uint8_t request_buf[128];
    pb_ostream_t ostream = pb_ostream_from_buffer(request_buf, sizeof(request_buf));
    if (!pb_encode(&ostream, demo_GetPersonRequest_fields, &req)) {
        fprintf(stderr, "Failed to encode request\n");
        return false;
    }
    
    // Call RPC
    uint8_t response_buf[512];
    size_t response_size;
    if (!rpc_client_call(client, RPC_METHOD_GET_PERSON,
                         request_buf, ostream.bytes_written,
                         response_buf, &response_size, sizeof(response_buf))) {
        fprintf(stderr, "RPC call failed\n");
        return false;
    }
    
    // Decode response
    demo_GetPersonResponse resp = demo_GetPersonResponse_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(response_buf, response_size);
    if (!pb_decode(&istream, demo_GetPersonResponse_fields, &resp)) {
        fprintf(stderr, "Failed to decode response\n");
        return false;
    }
    
    // Print result
    if (resp.found) {
        printf("Found person:\n");
        printf("  Name: %s\n", resp.person.name);
        printf("  ID: %d\n", resp.person.id);
        printf("  Email: %s\n", resp.person.email);
        printf("  Phones: %d\n", resp.person.phones_count);
        for (size_t i = 0; i < resp.person.phones_count; i++) {
            const char* type = "UNKNOWN";
            switch (resp.person.phones[i].type) {
                case demo_Person_PhoneType_MOBILE: type = "MOBILE"; break;
                case demo_Person_PhoneType_HOME: type = "HOME"; break;
                case demo_Person_PhoneType_WORK: type = "WORK"; break;
            }
            printf("    [%zu] %s (%s)\n", i, resp.person.phones[i].number, type);
        }
    } else {
        printf("Person not found\n");
    }
    
    return true;
}

// Call AddPerson RPC
static bool call_add_person(rpc_client_t* client, const char* name, int32_t id, const char* email) {
    printf("\n--- AddPerson(name=%s, id=%d, email=%s) ---\n", name, id, email);
    
    // Create request
    demo_AddPersonRequest req = demo_AddPersonRequest_init_zero;
    req.person.id = id;
    strncpy(req.person.name, name, sizeof(req.person.name) - 1);
    strncpy(req.person.email, email, sizeof(req.person.email) - 1);
    req.person.phones_count = 1;
    strncpy(req.person.phones[0].number, "555-3333", sizeof(req.person.phones[0].number) - 1);
    req.person.phones[0].type = demo_Person_PhoneType_MOBILE;
    
    // Encode request
    uint8_t request_buf[512];
    pb_ostream_t ostream = pb_ostream_from_buffer(request_buf, sizeof(request_buf));
    if (!pb_encode(&ostream, demo_AddPersonRequest_fields, &req)) {
        fprintf(stderr, "Failed to encode request\n");
        return false;
    }
    
    // Call RPC
    uint8_t response_buf[256];
    size_t response_size;
    if (!rpc_client_call(client, RPC_METHOD_ADD_PERSON,
                         request_buf, ostream.bytes_written,
                         response_buf, &response_size, sizeof(response_buf))) {
        fprintf(stderr, "RPC call failed\n");
        return false;
    }
    
    // Decode response
    demo_AddPersonResponse resp = demo_AddPersonResponse_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(response_buf, response_size);
    if (!pb_decode(&istream, demo_AddPersonResponse_fields, &resp)) {
        fprintf(stderr, "Failed to decode response\n");
        return false;
    }
    
    // Print result
    printf("Success: %s\n", resp.success ? "yes" : "no");
    printf("Message: %s\n", resp.message);
    
    return true;
}

// Call ListPersons RPC
static bool call_list_persons(rpc_client_t* client, int32_t max_results) {
    printf("\n--- ListPersons(max_results=%d) ---\n", max_results);
    
    // Create request
    demo_ListPersonsRequest req = demo_ListPersonsRequest_init_zero;
    req.max_results = max_results;
    
    // Encode request
    uint8_t request_buf[128];
    pb_ostream_t ostream = pb_ostream_from_buffer(request_buf, sizeof(request_buf));
    if (!pb_encode(&ostream, demo_ListPersonsRequest_fields, &req)) {
        fprintf(stderr, "Failed to encode request\n");
        return false;
    }
    
    // Call RPC
    uint8_t response_buf[4096];
    size_t response_size;
    if (!rpc_client_call(client, RPC_METHOD_LIST_PERSONS,
                         request_buf, ostream.bytes_written,
                         response_buf, &response_size, sizeof(response_buf))) {
        fprintf(stderr, "RPC call failed\n");
        return false;
    }
    
    // Decode response
    demo_ListPersonsResponse resp = demo_ListPersonsResponse_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(response_buf, response_size);
    if (!pb_decode(&istream, demo_ListPersonsResponse_fields, &resp)) {
        fprintf(stderr, "Failed to decode response\n");
        return false;
    }
    
    // Print result
    printf("Total persons: %d\n", resp.total_count);
    printf("Returned: %d\n", resp.persons_count);
    for (size_t i = 0; i < resp.persons_count; i++) {
        printf("  [%zu] %s (ID: %d, Email: %s)\n", 
               i, resp.persons[i].name, resp.persons[i].id, resp.persons[i].email);
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    const char* host = "127.0.0.1";
    uint16_t port = 8080;
    
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = (uint16_t)atoi(argv[2]);
    }
    
    printf("RPC Client Demo\n");
    printf("================\n");
    printf("Connecting to %s:%d\n", host, port);
    
    // Create client
    rpc_client_t* client = rpc_client_create(host, port);
    if (!client) {
        fprintf(stderr, "Failed to create client\n");
        return 1;
    }
    
    // Test GetPerson for existing person
    if (!call_get_person(client, 1)) {
        rpc_client_destroy(client);
        return 1;
    }
    
    // Test ListPersons
    if (!call_list_persons(client, 10)) {
        rpc_client_destroy(client);
        return 1;
    }
    
    // Test AddPerson
    if (!call_add_person(client, "Charlie", 3, "charlie@example.com")) {
        rpc_client_destroy(client);
        return 1;
    }
    
    // Test GetPerson for newly added person
    if (!call_get_person(client, 3)) {
        rpc_client_destroy(client);
        return 1;
    }
    
    // Test ListPersons again
    if (!call_list_persons(client, 10)) {
        rpc_client_destroy(client);
        return 1;
    }
    
    // Test GetPerson for non-existent person
    if (!call_get_person(client, 999)) {
        rpc_client_destroy(client);
        return 1;
    }
    
    // Test adding duplicate
    if (!call_add_person(client, "Charlie Again", 3, "charlie2@example.com")) {
        rpc_client_destroy(client);
        return 1;
    }
    
    printf("\n=== All RPC calls completed successfully! ===\n");
    
    rpc_client_destroy(client);
    return 0;
}
