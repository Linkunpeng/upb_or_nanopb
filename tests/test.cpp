#include <gtest/gtest.h>
#include <cstring>

// UPB includes
extern "C" {
#include "person.upb.h"
#include "upb/mem/arena.h"
#include "upb/encode.h"
#include "upb/decode.h"
}

// Nanopb includes
extern "C" {
#include "person.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
}

// Test UPB encoding and decoding
TEST(UPBTest, BasicEncodeDecode) {
    upb_Arena* arena = upb_Arena_New();
    ASSERT_NE(arena, nullptr);
    
    // Create a person
    demo_Person* person = demo_Person_new(arena);
    ASSERT_NE(person, nullptr);
    
    demo_Person_set_name(person, upb_StringView_FromString("Alice"));
    demo_Person_set_id(person, 42);
    demo_Person_set_email(person, upb_StringView_FromString("alice@example.com"));
    
    // Encode
    size_t size;
    char* encoded = demo_Person_serialize(person, arena, &size);
    ASSERT_NE(encoded, nullptr);
    ASSERT_GT(size, 0u);
    
    // Decode
    demo_Person* decoded = demo_Person_parse(encoded, size, arena);
    ASSERT_NE(decoded, nullptr);
    
    // Verify
    upb_StringView name = demo_Person_name(decoded);
    EXPECT_EQ(std::string(name.data, name.size), "Alice");
    EXPECT_EQ(demo_Person_id(decoded), 42);
    
    upb_StringView email = demo_Person_email(decoded);
    EXPECT_EQ(std::string(email.data, email.size), "alice@example.com");
    
    upb_Arena_Free(arena);
}

TEST(UPBTest, RepeatedFields) {
    upb_Arena* arena = upb_Arena_New();
    ASSERT_NE(arena, nullptr);
    
    demo_Person* person = demo_Person_new(arena);
    
    // Add phone numbers
    demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, arena);
    demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("111-2222"));
    demo_Person_PhoneNumber_set_type(phone1, demo_Person_MOBILE);
    
    demo_Person_PhoneNumber* phone2 = demo_Person_add_phones(person, arena);
    demo_Person_PhoneNumber_set_number(phone2, upb_StringView_FromString("333-4444"));
    demo_Person_PhoneNumber_set_type(phone2, demo_Person_WORK);
    
    // Encode and decode
    size_t size;
    char* encoded = demo_Person_serialize(person, arena, &size);
    demo_Person* decoded = demo_Person_parse(encoded, size, arena);
    
    // Verify
    size_t phone_count;
    const demo_Person_PhoneNumber* const* phones = demo_Person_phones(decoded, &phone_count);
    EXPECT_EQ(phone_count, 2u);
    
    upb_StringView num1 = demo_Person_PhoneNumber_number(phones[0]);
    EXPECT_EQ(std::string(num1.data, num1.size), "111-2222");
    EXPECT_EQ(demo_Person_PhoneNumber_type(phones[0]), demo_Person_MOBILE);
    
    upb_StringView num2 = demo_Person_PhoneNumber_number(phones[1]);
    EXPECT_EQ(std::string(num2.data, num2.size), "333-4444");
    EXPECT_EQ(demo_Person_PhoneNumber_type(phones[1]), demo_Person_WORK);
    
    upb_Arena_Free(arena);
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

// Cross-compatibility test: Verify that messages encoded with UPB can be decoded with Nanopb
TEST(CompatibilityTest, UPBEncodeNanopbDecode) {
    upb_Arena* arena = upb_Arena_New();
    
    // Encode with UPB
    demo_Person* upb_person = demo_Person_new(arena);
    demo_Person_set_name(upb_person, upb_StringView_FromString("Cross"));
    demo_Person_set_id(upb_person, 123);
    demo_Person_set_email(upb_person, upb_StringView_FromString("cross@test.com"));
    
    size_t size;
    char* encoded = demo_Person_serialize(upb_person, arena, &size);
    ASSERT_NE(encoded, nullptr);
    
    // Decode with Nanopb
    demo_Person nanopb_person = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer((uint8_t*)encoded, size);
    bool decode_status = pb_decode(&istream, demo_Person_fields, &nanopb_person);
    ASSERT_TRUE(decode_status);
    
    // Verify
    EXPECT_STREQ(nanopb_person.name, "Cross");
    EXPECT_EQ(nanopb_person.id, 123);
    EXPECT_STREQ(nanopb_person.email, "cross@test.com");
    
    upb_Arena_Free(arena);
}

// Cross-compatibility test: Verify that messages encoded with Nanopb can be decoded with UPB
TEST(CompatibilityTest, NanopbEncodeUPBDecode) {
    // Encode with Nanopb
    demo_Person nanopb_person = demo_Person_init_zero;
    strcpy(nanopb_person.name, "Reverse");
    nanopb_person.id = 456;
    strcpy(nanopb_person.email, "reverse@test.com");
    
    uint8_t buffer[256];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    ASSERT_TRUE(pb_encode(&ostream, demo_Person_fields, &nanopb_person));
    
    // Decode with UPB
    upb_Arena* arena = upb_Arena_New();
    demo_Person* upb_person = demo_Person_parse((char*)buffer, ostream.bytes_written, arena);
    ASSERT_NE(upb_person, nullptr);
    
    // Verify
    upb_StringView name = demo_Person_name(upb_person);
    EXPECT_EQ(std::string(name.data, name.size), "Reverse");
    EXPECT_EQ(demo_Person_id(upb_person), 456);
    
    upb_StringView email = demo_Person_email(upb_person);
    EXPECT_EQ(std::string(email.data, email.size), "reverse@test.com");
    
    upb_Arena_Free(arena);
}
