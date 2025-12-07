#include <stdio.h>
#include <string.h>
#include "person.upb.h"
#include "upb/mem/arena.h"
#include "upb/encode.h"
#include "upb/decode.h"

void create_sample_person(upb_Arena* arena, demo_Person* person) {
    demo_Person_set_name(person, upb_StringView_FromString("John Doe"));
    demo_Person_set_id(person, 12345);
    demo_Person_set_email(person, upb_StringView_FromString("john.doe@example.com"));
    
    // Add phone numbers
    demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, arena);
    demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("555-1234"));
    demo_Person_PhoneNumber_set_type(phone1, demo_Person_MOBILE);
    
    demo_Person_PhoneNumber* phone2 = demo_Person_add_phones(person, arena);
    demo_Person_PhoneNumber_set_number(phone2, upb_StringView_FromString("555-5678"));
    demo_Person_PhoneNumber_set_type(phone2, demo_Person_WORK);
}

int main() {
    printf("UPB Demo\n");
    printf("========\n\n");
    
    // Create arena for memory management
    upb_Arena* arena = upb_Arena_New();
    
    // Create and populate a Person message
    demo_Person* person = demo_Person_new(arena);
    create_sample_person(arena, person);
    
    // Encode the message
    size_t size;
    char* encoded = demo_Person_serialize(person, arena, &size);
    
    if (!encoded) {
        printf("Error: Failed to encode message\n");
        upb_Arena_Free(arena);
        return 1;
    }
    
    printf("Encoded message size: %zu bytes\n", size);
    
    // Decode the message back
    demo_Person* decoded_person = demo_Person_parse(encoded, size, arena);
    
    if (!decoded_person) {
        printf("Error: Failed to decode message\n");
        upb_Arena_Free(arena);
        return 1;
    }
    
    // Print decoded values
    printf("\nDecoded Person:\n");
    printf("  Name: %.*s\n", 
           (int)demo_Person_name(decoded_person).size,
           demo_Person_name(decoded_person).data);
    printf("  ID: %d\n", demo_Person_id(decoded_person));
    printf("  Email: %.*s\n",
           (int)demo_Person_email(decoded_person).size,
           demo_Person_email(decoded_person).data);
    
    // Print phone numbers
    size_t phone_count;
    const demo_Person_PhoneNumber* const* phones = 
        demo_Person_phones(decoded_person, &phone_count);
    
    printf("  Phones (%zu):\n", phone_count);
    for (size_t i = 0; i < phone_count; i++) {
        upb_StringView number = demo_Person_PhoneNumber_number(phones[i]);
        demo_Person_PhoneType type = demo_Person_PhoneNumber_type(phones[i]);
        const char* type_str = (type == demo_Person_MOBILE) ? "MOBILE" :
                               (type == demo_Person_HOME) ? "HOME" : "WORK";
        printf("    [%zu] %.*s (%s)\n", i, (int)number.size, number.data, type_str);
    }
    
    printf("\nUPB demo completed successfully!\n");
    
    // Cleanup
    upb_Arena_Free(arena);
    
    return 0;
}
