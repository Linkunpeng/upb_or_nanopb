#include <gtest/gtest.h>
#include <cstring>
#include <string>

// UPB includes
extern "C" {
#include "person.upb.h"
#include "upb/mem/arena.h"
#include "upb/encode.h"
#include "upb/decode.h"
}

// Nanopb includes - wrapped to avoid conflicts
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

// Cross-compatibility test: Verify that messages encoded with UPB can be decoded with Nanopb
TEST(CompatibilityTest, UpbEncodeNanopbDecode) {
    // Encode with UPB
    upb_Arena* arena = upb_Arena_New();
    ASSERT_NE(arena, nullptr);
    
    demo_Person* upb_person = demo_Person_new(arena);
    demo_Person_set_name(upb_person, upb_StringView_FromString("Cross"));
    demo_Person_set_id(upb_person, 123);
    demo_Person_set_email(upb_person, upb_StringView_FromString("cross@test.com"));
    
    size_t size;
    char* encoded = demo_Person_serialize(upb_person, arena, &size);
    ASSERT_NE(encoded, nullptr);
    
    // Decode with Nanopb
    demo_Person_nanopb nanopb_person = {};
    bool decode_status = nanopb_decode_person((const uint8_t*)encoded, size, &nanopb_person);
    ASSERT_TRUE(decode_status);
    
    // Verify
    EXPECT_STREQ(nanopb_person.name, "Cross");
    EXPECT_EQ(nanopb_person.id, 123);
    EXPECT_STREQ(nanopb_person.email, "cross@test.com");
    
    upb_Arena_Free(arena);
}

// Cross-compatibility test: Verify that messages encoded with Nanopb can be decoded with UPB
TEST(CompatibilityTest, NanopbEncodeUpbDecode) {
    // Encode with Nanopb
    demo_Person_nanopb nanopb_person = {};
    strncpy(nanopb_person.name, "Reverse", sizeof(nanopb_person.name) - 1);
    nanopb_person.id = 456;
    strncpy(nanopb_person.email, "reverse@test.com", sizeof(nanopb_person.email) - 1);
    
    uint8_t buffer[256];
    size_t bytes_written;
    ASSERT_TRUE(nanopb_encode_person(&nanopb_person, buffer, sizeof(buffer), &bytes_written));
    
    // Decode with UPB
    upb_Arena* arena = upb_Arena_New();
    ASSERT_NE(arena, nullptr);
    
    demo_Person* upb_person = demo_Person_parse((char*)buffer, bytes_written, arena);
    ASSERT_NE(upb_person, nullptr);
    
    // Verify
    upb_StringView name = demo_Person_name(upb_person);
    EXPECT_EQ(std::string(name.data, name.size), "Reverse");
    EXPECT_EQ(demo_Person_id(upb_person), 456);
    
    upb_StringView email = demo_Person_email(upb_person);
    EXPECT_EQ(std::string(email.data, email.size), "reverse@test.com");
    
    upb_Arena_Free(arena);
}

// Test with repeated fields (UPB to Nanopb)
TEST(CompatibilityTest, RepeatedFieldsUpbToNanopb) {
    upb_Arena* arena = upb_Arena_New();
    ASSERT_NE(arena, nullptr);
    
    // Create UPB message with phones
    demo_Person* upb_person = demo_Person_new(arena);
    demo_Person_set_name(upb_person, upb_StringView_FromString("PhoneTest"));
    demo_Person_set_id(upb_person, 789);
    
    demo_Person_PhoneNumber* phone = demo_Person_add_phones(upb_person, arena);
    demo_Person_PhoneNumber_set_number(phone, upb_StringView_FromString("555-1111"));
    demo_Person_PhoneNumber_set_type(phone, demo_Person_PhoneType_MOBILE);
    
    // Encode with UPB
    size_t size;
    char* encoded = demo_Person_serialize(upb_person, arena, &size);
    ASSERT_NE(encoded, nullptr);
    
    // Decode with Nanopb
    demo_Person_nanopb nanopb_person = {};
    ASSERT_TRUE(nanopb_decode_person((const uint8_t*)encoded, size, &nanopb_person));
    
    // Verify
    EXPECT_EQ(nanopb_person.id, 789);
    EXPECT_EQ(nanopb_person.phones_count, 1u);
    EXPECT_STREQ(nanopb_person.phones[0].number, "555-1111");
    EXPECT_EQ(nanopb_person.phones[0].type, 0);  // MOBILE
    
    upb_Arena_Free(arena);
}

// Test with repeated fields (Nanopb to UPB)
TEST(CompatibilityTest, RepeatedFieldsNanopbToUpb) {
    // Create Nanopb message with phones
    demo_Person_nanopb nanopb_person = {};
    strncpy(nanopb_person.name, "PhoneTest2", sizeof(nanopb_person.name) - 1);
    nanopb_person.id = 999;
    
    nanopb_person.phones_count = 2;
    strncpy(nanopb_person.phones[0].number, "555-2222", sizeof(nanopb_person.phones[0].number) - 1);
    nanopb_person.phones[0].type = 1;  // HOME
    
    strncpy(nanopb_person.phones[1].number, "555-3333", sizeof(nanopb_person.phones[1].number) - 1);
    nanopb_person.phones[1].type = 2;  // WORK
    
    // Encode with Nanopb
    uint8_t buffer[256];
    size_t bytes_written;
    ASSERT_TRUE(nanopb_encode_person(&nanopb_person, buffer, sizeof(buffer), &bytes_written));
    
    // Decode with UPB
    upb_Arena* arena = upb_Arena_New();
    ASSERT_NE(arena, nullptr);
    
    demo_Person* upb_person = demo_Person_parse((char*)buffer, bytes_written, arena);
    ASSERT_NE(upb_person, nullptr);
    
    // Verify
    EXPECT_EQ(demo_Person_id(upb_person), 999);
    
    size_t phone_count;
    const demo_Person_PhoneNumber* const* phones = demo_Person_phones(upb_person, &phone_count);
    EXPECT_EQ(phone_count, 2u);
    
    upb_StringView num1 = demo_Person_PhoneNumber_number(phones[0]);
    EXPECT_EQ(std::string(num1.data, num1.size), "555-2222");
    EXPECT_EQ(demo_Person_PhoneNumber_type(phones[0]), demo_Person_PhoneType_HOME);
    
    upb_StringView num2 = demo_Person_PhoneNumber_number(phones[1]);
    EXPECT_EQ(std::string(num2.data, num2.size), "555-3333");
    EXPECT_EQ(demo_Person_PhoneNumber_type(phones[1]), demo_Person_PhoneType_WORK);
    
    upb_Arena_Free(arena);
}
