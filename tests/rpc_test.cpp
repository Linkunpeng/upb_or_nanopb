#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <unistd.h>

extern "C" {
#include "simple_rpc.h"
#include "person.pb.h"
#include "person_service.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
}

// Test port (using higher port to avoid conflicts)
constexpr uint16_t TEST_PORT = 18080;

// Simple echo handler for testing
static bool echo_handler(const uint8_t* request, size_t request_size,
                         uint8_t* response, size_t* response_size,
                         size_t response_max) {
    if (request_size > response_max) {
        return false;
    }
    memcpy(response, request, request_size);
    *response_size = request_size;
    return true;
}

// Handler that returns a fixed GetPersonResponse
static bool test_get_person_handler(const uint8_t* request, size_t request_size,
                                     uint8_t* response, size_t* response_size,
                                     size_t response_max) {
    // Decode request
    demo_GetPersonRequest req = demo_GetPersonRequest_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(request, request_size);
    
    if (!pb_decode(&istream, demo_GetPersonRequest_fields, &req)) {
        return false;
    }
    
    // Create response
    demo_GetPersonResponse resp = demo_GetPersonResponse_init_zero;
    if (req.id == 123) {
        resp.found = true;
        resp.person.id = 123;
        strncpy(resp.person.name, "Test User", sizeof(resp.person.name) - 1);
        strncpy(resp.person.email, "test@example.com", sizeof(resp.person.email) - 1);
        resp.person.phones_count = 1;
        strncpy(resp.person.phones[0].number, "555-TEST", sizeof(resp.person.phones[0].number) - 1);
        resp.person.phones[0].type = demo_Person_PhoneType_MOBILE;
    } else {
        resp.found = false;
    }
    
    // Encode response
    pb_ostream_t ostream = pb_ostream_from_buffer(response, response_max);
    if (!pb_encode(&ostream, demo_GetPersonResponse_fields, &resp)) {
        return false;
    }
    
    *response_size = ostream.bytes_written;
    return true;
}

class RPCTest : public ::testing::Test {
protected:
    rpc_server_t* server = nullptr;
    rpc_client_t* client = nullptr;
    std::thread* server_thread = nullptr;
    
    void SetUp() override {
        // Create server
        server = rpc_server_create(TEST_PORT);
        ASSERT_NE(server, nullptr);
        
        // Register test handler
        ASSERT_TRUE(rpc_server_register_handler(server, RPC_METHOD_GET_PERSON, test_get_person_handler));
        
        // Start server
        ASSERT_TRUE(rpc_server_start(server));
        
        // Run server in background thread
        server_thread = new std::thread([this]() {
            while (this->server) {
                rpc_server_handle_request(this->server);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        
        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Create client
        client = rpc_client_create("127.0.0.1", TEST_PORT);
        ASSERT_NE(client, nullptr);
    }
    
    void TearDown() override {
        // Cleanup client
        if (client) {
            rpc_client_destroy(client);
            client = nullptr;
        }
        
        // Cleanup server
        if (server) {
            rpc_server_stop(server);
            auto temp_server = server;
            server = nullptr;  // Signal thread to stop
            
            if (server_thread) {
                server_thread->join();
                delete server_thread;
                server_thread = nullptr;
            }
            
            rpc_server_destroy(temp_server);
        }
        
        // Give system time to release port
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
};

TEST_F(RPCTest, ServerCreation) {
    // Server already created in SetUp
    EXPECT_NE(server, nullptr);
}

TEST_F(RPCTest, ClientCreation) {
    // Client already created in SetUp
    EXPECT_NE(client, nullptr);
}

TEST_F(RPCTest, ClientConnect) {
    EXPECT_TRUE(rpc_client_connect(client));
    rpc_client_disconnect(client);
}

TEST_F(RPCTest, GetPersonFound) {
    // Create request for existing person
    demo_GetPersonRequest req = demo_GetPersonRequest_init_zero;
    req.id = 123;
    
    // Encode request
    uint8_t request_buf[128];
    pb_ostream_t ostream = pb_ostream_from_buffer(request_buf, sizeof(request_buf));
    ASSERT_TRUE(pb_encode(&ostream, demo_GetPersonRequest_fields, &req));
    
    // Call RPC
    uint8_t response_buf[512];
    size_t response_size;
    ASSERT_TRUE(rpc_client_call(client, RPC_METHOD_GET_PERSON,
                                 request_buf, ostream.bytes_written,
                                 response_buf, &response_size, sizeof(response_buf)));
    
    // Decode response
    demo_GetPersonResponse resp = demo_GetPersonResponse_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(response_buf, response_size);
    ASSERT_TRUE(pb_decode(&istream, demo_GetPersonResponse_fields, &resp));
    
    // Verify
    EXPECT_TRUE(resp.found);
    EXPECT_EQ(resp.person.id, 123);
    EXPECT_STREQ(resp.person.name, "Test User");
    EXPECT_STREQ(resp.person.email, "test@example.com");
    EXPECT_EQ(resp.person.phones_count, 1);
    EXPECT_STREQ(resp.person.phones[0].number, "555-TEST");
    EXPECT_EQ(resp.person.phones[0].type, demo_Person_PhoneType_MOBILE);
}

TEST_F(RPCTest, GetPersonNotFound) {
    // Create request for non-existent person
    demo_GetPersonRequest req = demo_GetPersonRequest_init_zero;
    req.id = 999;
    
    // Encode request
    uint8_t request_buf[128];
    pb_ostream_t ostream = pb_ostream_from_buffer(request_buf, sizeof(request_buf));
    ASSERT_TRUE(pb_encode(&ostream, demo_GetPersonRequest_fields, &req));
    
    // Call RPC
    uint8_t response_buf[512];
    size_t response_size;
    ASSERT_TRUE(rpc_client_call(client, RPC_METHOD_GET_PERSON,
                                 request_buf, ostream.bytes_written,
                                 response_buf, &response_size, sizeof(response_buf)));
    
    // Decode response
    demo_GetPersonResponse resp = demo_GetPersonResponse_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(response_buf, response_size);
    ASSERT_TRUE(pb_decode(&istream, demo_GetPersonResponse_fields, &resp));
    
    // Verify
    EXPECT_FALSE(resp.found);
}

TEST_F(RPCTest, MultipleRequests) {
    // Make multiple requests in sequence
    for (int i = 0; i < 5; i++) {
        demo_GetPersonRequest req = demo_GetPersonRequest_init_zero;
        req.id = 123;
        
        uint8_t request_buf[128];
        pb_ostream_t ostream = pb_ostream_from_buffer(request_buf, sizeof(request_buf));
        ASSERT_TRUE(pb_encode(&ostream, demo_GetPersonRequest_fields, &req));
        
        uint8_t response_buf[512];
        size_t response_size;
        ASSERT_TRUE(rpc_client_call(client, RPC_METHOD_GET_PERSON,
                                     request_buf, ostream.bytes_written,
                                     response_buf, &response_size, sizeof(response_buf)));
        
        demo_GetPersonResponse resp = demo_GetPersonResponse_init_zero;
        pb_istream_t istream = pb_istream_from_buffer(response_buf, response_size);
        ASSERT_TRUE(pb_decode(&istream, demo_GetPersonResponse_fields, &resp));
        
        EXPECT_TRUE(resp.found);
    }
}

TEST_F(RPCTest, NanopbSerialization) {
    // Test that nanopb serialization/deserialization works correctly
    demo_Person person = demo_Person_init_zero;
    person.id = 456;
    strncpy(person.name, "Nanopb Test", sizeof(person.name) - 1);
    strncpy(person.email, "nanopb@test.com", sizeof(person.email) - 1);
    person.phones_count = 2;
    strncpy(person.phones[0].number, "111-2222", sizeof(person.phones[0].number) - 1);
    person.phones[0].type = demo_Person_PhoneType_HOME;
    strncpy(person.phones[1].number, "333-4444", sizeof(person.phones[1].number) - 1);
    person.phones[1].type = demo_Person_PhoneType_WORK;
    
    // Encode
    uint8_t buffer[512];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    ASSERT_TRUE(pb_encode(&ostream, demo_Person_fields, &person));
    size_t encoded_size = ostream.bytes_written;
    
    // Decode
    demo_Person decoded = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(buffer, encoded_size);
    ASSERT_TRUE(pb_decode(&istream, demo_Person_fields, &decoded));
    
    // Verify
    EXPECT_EQ(decoded.id, person.id);
    EXPECT_STREQ(decoded.name, person.name);
    EXPECT_STREQ(decoded.email, person.email);
    EXPECT_EQ(decoded.phones_count, person.phones_count);
    for (size_t i = 0; i < person.phones_count; i++) {
        EXPECT_STREQ(decoded.phones[i].number, person.phones[i].number);
        EXPECT_EQ(decoded.phones[i].type, person.phones[i].type);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
