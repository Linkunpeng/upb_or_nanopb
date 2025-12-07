#include <gtest/gtest.h>
#include <cstring>

// Nanopb includes
extern "C" {
#include "person.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
}

// Test Nanopb encoding and decoding
TEST(NanopbTest, BasicEncodeDecode) {
    demo_Person person = demo_Person_init_zero;
    strcpy(person.name, "Bob");
    person.id = 99;
    strcpy(person.email, "bob@example.com");
    
    // Encode
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool encode_status = pb_encode(&stream, demo_Person_fields, &person);
    ASSERT_TRUE(encode_status);
    
    size_t encoded_size = stream.bytes_written;
    ASSERT_GT(encoded_size, 0u);
    
    // Decode
    demo_Person decoded = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(buffer, encoded_size);
    bool decode_status = pb_decode(&istream, demo_Person_fields, &decoded);
    ASSERT_TRUE(decode_status);
    
    // Verify
    EXPECT_STREQ(decoded.name, "Bob");
    EXPECT_EQ(decoded.id, 99);
    EXPECT_STREQ(decoded.email, "bob@example.com");
}

TEST(NanopbTest, RepeatedFields) {
    demo_Person person = demo_Person_init_zero;
    strcpy(person.name, "Charlie");
    person.id = 77;
    
    person.phones_count = 2;
    strcpy(person.phones[0].number, "555-0000");
    person.phones[0].type = demo_Person_PhoneType_HOME;
    strcpy(person.phones[1].number, "555-9999");
    person.phones[1].type = demo_Person_PhoneType_MOBILE;
    
    // Encode
    uint8_t buffer[256];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    ASSERT_TRUE(pb_encode(&ostream, demo_Person_fields, &person));
    
    // Decode
    demo_Person decoded = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(buffer, ostream.bytes_written);
    ASSERT_TRUE(pb_decode(&istream, demo_Person_fields, &decoded));
    
    // Verify
    EXPECT_EQ(decoded.phones_count, 2u);
    EXPECT_STREQ(decoded.phones[0].number, "555-0000");
    EXPECT_EQ(decoded.phones[0].type, demo_Person_PhoneType_HOME);
    EXPECT_STREQ(decoded.phones[1].number, "555-9999");
    EXPECT_EQ(decoded.phones[1].type, demo_Person_PhoneType_MOBILE);
}

TEST(NanopbTest, EmptyMessage) {
    demo_Person person = demo_Person_init_zero;
    
    // Encode empty message
    uint8_t buffer[256];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    ASSERT_TRUE(pb_encode(&ostream, demo_Person_fields, &person));
    
    size_t encoded_size = ostream.bytes_written;
    // Empty proto3 messages can encode to 0 bytes
    EXPECT_GE(encoded_size, 0u);
    
    // Decode
    demo_Person decoded = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(buffer, encoded_size);
    ASSERT_TRUE(pb_decode(&istream, demo_Person_fields, &decoded));
}

TEST(NanopbTest, MaxFields) {
    demo_Person person = demo_Person_init_zero;
    
    // Fill with maximum data
    memset(person.name, 'A', sizeof(person.name) - 1);
    person.name[sizeof(person.name) - 1] = '\0';
    person.id = INT32_MAX;
    memset(person.email, 'B', sizeof(person.email) - 1);
    person.email[sizeof(person.email) - 1] = '\0';
    
    person.phones_count = 10;  // Max count from .options file
    for (size_t i = 0; i < person.phones_count; i++) {
        snprintf(person.phones[i].number, sizeof(person.phones[i].number), "555-%04zu", i);
        person.phones[i].type = (demo_Person_PhoneType)(i % 3);
    }
    
    // Encode
    uint8_t buffer[1024];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    ASSERT_TRUE(pb_encode(&ostream, demo_Person_fields, &person));
    
    // Decode
    demo_Person decoded = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(buffer, ostream.bytes_written);
    ASSERT_TRUE(pb_decode(&istream, demo_Person_fields, &decoded));
    
    // Verify
    EXPECT_STREQ(decoded.name, person.name);
    EXPECT_EQ(decoded.id, INT32_MAX);
    EXPECT_EQ(decoded.phones_count, 10u);
}
