#include <gtest/gtest.h>
#include <cstring>
#include <string>

// Include C++ Protobuf
#include "person.pb.h"

// For nanopb, we need to wrap it to avoid naming conflicts
extern "C" {
    // Forward declarations for nanopb types
    typedef struct {
        char name[64];
        int32_t id;
        char email[64];
        uint32_t phones_count;
        struct {
            char number[32];
            int type;
        } phones[10];
    } demo_Person_nanopb;
    
    // Forward declare nanopb functions
    bool nanopb_encode_person(const demo_Person_nanopb* person, uint8_t* buffer, size_t buffer_size, size_t* bytes_written);
    bool nanopb_decode_person(const uint8_t* buffer, size_t size, demo_Person_nanopb* person);
}

// Cross-compatibility test: Verify that messages encoded with C++ Protobuf can be decoded with Nanopb
TEST(CompatibilityTest, CppPbEncodeNanopbDecode) {
    // Encode with C++ Protobuf
    demo::Person cpp_person;
    cpp_person.set_name("Cross");
    cpp_person.set_id(123);
    cpp_person.set_email("cross@test.com");
    
    std::string encoded;
    ASSERT_TRUE(cpp_person.SerializeToString(&encoded));
    
    // Decode with Nanopb
    demo_Person_nanopb nanopb_person = {};
    bool decode_status = nanopb_decode_person((const uint8_t*)encoded.data(), encoded.size(), &nanopb_person);
    ASSERT_TRUE(decode_status);
    
    // Verify
    EXPECT_STREQ(nanopb_person.name, "Cross");
    EXPECT_EQ(nanopb_person.id, 123);
    EXPECT_STREQ(nanopb_person.email, "cross@test.com");
}

// Cross-compatibility test: Verify that messages encoded with Nanopb can be decoded with C++ Protobuf
TEST(CompatibilityTest, NanopbEncodeCppPbDecode) {
    // Encode with Nanopb
    demo_Person_nanopb nanopb_person = {};
    strncpy(nanopb_person.name, "Reverse", sizeof(nanopb_person.name) - 1);
    nanopb_person.id = 456;
    strncpy(nanopb_person.email, "reverse@test.com", sizeof(nanopb_person.email) - 1);
    
    uint8_t buffer[256];
    size_t bytes_written;
    ASSERT_TRUE(nanopb_encode_person(&nanopb_person, buffer, sizeof(buffer), &bytes_written));
    
    // Decode with C++ Protobuf
    demo::Person cpp_person;
    ASSERT_TRUE(cpp_person.ParseFromArray(buffer, bytes_written));
    
    // Verify
    EXPECT_EQ(cpp_person.name(), "Reverse");
    EXPECT_EQ(cpp_person.id(), 456);
    EXPECT_EQ(cpp_person.email(), "reverse@test.com");
}
