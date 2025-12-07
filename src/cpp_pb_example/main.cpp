#include <iostream>
#include <string>
#include "person.pb.h"

void create_sample_person(demo::Person* person) {
    // Set basic fields
    person->set_name("John Doe");
    person->set_id(12345);
    person->set_email("john.doe@example.com");
    
    // Add phone numbers
    demo::Person::PhoneNumber* phone1 = person->add_phones();
    phone1->set_number("555-1234");
    phone1->set_type(demo::Person::MOBILE);
    
    demo::Person::PhoneNumber* phone2 = person->add_phones();
    phone2->set_number("555-5678");
    phone2->set_type(demo::Person::WORK);
}

int main() {
    std::cout << "C++ Protobuf Demo\n";
    std::cout << "==================\n\n";
    
    // Create and populate a Person message
    demo::Person person;
    create_sample_person(&person);
    
    // Encode the message
    std::string encoded;
    if (!person.SerializeToString(&encoded)) {
        std::cerr << "Error: Failed to encode message\n";
        return 1;
    }
    
    std::cout << "Encoded message size: " << encoded.size() << " bytes\n";
    
    // Decode the message back
    demo::Person decoded_person;
    if (!decoded_person.ParseFromString(encoded)) {
        std::cerr << "Error: Failed to decode message\n";
        return 1;
    }
    
    // Print decoded values
    std::cout << "\nDecoded Person:\n";
    std::cout << "  Name: " << decoded_person.name() << "\n";
    std::cout << "  ID: " << decoded_person.id() << "\n";
    std::cout << "  Email: " << decoded_person.email() << "\n";
    
    // Print phone numbers
    std::cout << "  Phones (" << decoded_person.phones_size() << "):\n";
    for (int i = 0; i < decoded_person.phones_size(); i++) {
        const auto& phone = decoded_person.phones(i);
        const char* type_str = (phone.type() == demo::Person::MOBILE) ? "MOBILE" :
                               (phone.type() == demo::Person::HOME) ? "HOME" : "WORK";
        std::cout << "    [" << i << "] " << phone.number() << " (" << type_str << ")\n";
    }
    
    std::cout << "\nC++ Protobuf demo completed successfully!\n";
    
    return 0;
}
