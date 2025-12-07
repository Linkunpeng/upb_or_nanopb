#include <gtest/gtest.h>
#include <string>
#include "person.pb.h"

// Test C++ Protobuf encoding and decoding
TEST(CppPbTest, BasicEncodeDecode) {
    // Create a person
    demo::Person person;
    person.set_name("Alice");
    person.set_id(42);
    person.set_email("alice@example.com");
    
    // Encode
    std::string encoded;
    ASSERT_TRUE(person.SerializeToString(&encoded));
    ASSERT_GT(encoded.size(), 0u);
    
    // Decode
    demo::Person decoded;
    ASSERT_TRUE(decoded.ParseFromString(encoded));
    
    // Verify
    EXPECT_EQ(decoded.name(), "Alice");
    EXPECT_EQ(decoded.id(), 42);
    EXPECT_EQ(decoded.email(), "alice@example.com");
}

TEST(CppPbTest, RepeatedFields) {
    demo::Person person;
    person.set_name("Bob");
    person.set_id(77);
    
    // Add phone numbers
    auto* phone1 = person.add_phones();
    phone1->set_number("111-2222");
    phone1->set_type(demo::Person::MOBILE);
    
    auto* phone2 = person.add_phones();
    phone2->set_number("333-4444");
    phone2->set_type(demo::Person::WORK);
    
    // Encode and decode
    std::string encoded;
    ASSERT_TRUE(person.SerializeToString(&encoded));
    
    demo::Person decoded;
    ASSERT_TRUE(decoded.ParseFromString(encoded));
    
    // Verify
    EXPECT_EQ(decoded.phones_size(), 2);
    EXPECT_EQ(decoded.phones(0).number(), "111-2222");
    EXPECT_EQ(decoded.phones(0).type(), demo::Person::MOBILE);
    EXPECT_EQ(decoded.phones(1).number(), "333-4444");
    EXPECT_EQ(decoded.phones(1).type(), demo::Person::WORK);
}

TEST(CppPbTest, EmptyMessage) {
    demo::Person person;
    
    // Encode empty message
    std::string encoded;
    ASSERT_TRUE(person.SerializeToString(&encoded));
    EXPECT_GE(encoded.size(), 0u);
    
    // Decode
    demo::Person decoded;
    ASSERT_TRUE(decoded.ParseFromString(encoded));
}

TEST(CppPbTest, LargeMessage) {
    demo::Person person;
    
    // Create a large message
    person.set_name(std::string(100, 'A'));
    person.set_id(INT32_MAX);
    person.set_email(std::string(100, 'B'));
    
    // Add multiple phones
    for (int i = 0; i < 10; i++) {
        auto* phone = person.add_phones();
        phone->set_number("555-" + std::to_string(i));
        phone->set_type(static_cast<demo::Person::PhoneType>(i % 3));
    }
    
    // Encode
    std::string encoded;
    ASSERT_TRUE(person.SerializeToString(&encoded));
    ASSERT_GT(encoded.size(), 0u);
    
    // Decode
    demo::Person decoded;
    ASSERT_TRUE(decoded.ParseFromString(encoded));
    
    // Verify
    EXPECT_EQ(decoded.name(), person.name());
    EXPECT_EQ(decoded.id(), INT32_MAX);
    EXPECT_EQ(decoded.phones_size(), 10);
}
