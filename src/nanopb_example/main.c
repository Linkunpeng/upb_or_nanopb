#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "person.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

void create_sample_person(demo_Person* person) {
    // Set basic fields
    strcpy(person->name, "John Doe");
    person->id = 12345;
    strcpy(person->email, "john.doe@example.com");
    
    // Add phone numbers
    person->phones_count = 2;
    
    strcpy(person->phones[0].number, "555-1234");
    person->phones[0].type = demo_Person_PhoneType_MOBILE;
    
    strcpy(person->phones[1].number, "555-5678");
    person->phones[1].type = demo_Person_PhoneType_WORK;
}

int main() {
    printf("Nanopb Demo\n");
    printf("===========\n\n");
    
    // Create and populate a Person message
    demo_Person person = demo_Person_init_zero;
    create_sample_person(&person);
    
    // Allocate buffer for encoding
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    
    // Encode the message
    bool encode_status = pb_encode(&stream, demo_Person_fields, &person);
    
    if (!encode_status) {
        printf("Error: Failed to encode message: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }
    
    size_t encoded_size = stream.bytes_written;
    printf("Encoded message size: %zu bytes\n", encoded_size);
    
    // Decode the message back
    demo_Person decoded_person = demo_Person_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(buffer, encoded_size);
    
    bool decode_status = pb_decode(&istream, demo_Person_fields, &decoded_person);
    
    if (!decode_status) {
        printf("Error: Failed to decode message: %s\n", PB_GET_ERROR(&istream));
        return 1;
    }
    
    // Print decoded values
    printf("\nDecoded Person:\n");
    printf("  Name: %s\n", decoded_person.name);
    printf("  ID: %d\n", decoded_person.id);
    printf("  Email: %s\n", decoded_person.email);
    
    // Print phone numbers
    printf("  Phones (%u):\n", decoded_person.phones_count);
    for (size_t i = 0; i < decoded_person.phones_count; i++) {
        const char* type_str = (decoded_person.phones[i].type == demo_Person_PhoneType_MOBILE) ? "MOBILE" :
                               (decoded_person.phones[i].type == demo_Person_PhoneType_HOME) ? "HOME" : "WORK";
        printf("    [%zu] %s (%s)\n", i, decoded_person.phones[i].number, type_str);
    }
    
    printf("\nNanopb demo completed successfully!\n");
    
    return 0;
}
