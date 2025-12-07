#include <cstring>

extern "C" {
#include "person.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
}

// Wrapper struct to match the test expectations
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

// Alias for benchmarking
typedef demo_Person_nanopb nanopb_Person;

extern "C" bool nanopb_encode_person(const demo_Person_nanopb* person_wrapper, uint8_t* buffer, size_t buffer_size, size_t* bytes_written) {
    // Convert wrapper to real nanopb struct
    demo_Person person = demo_Person_init_zero;
    strncpy(person.name, person_wrapper->name, sizeof(person.name) - 1);
    person.id = person_wrapper->id;
    strncpy(person.email, person_wrapper->email, sizeof(person.email) - 1);
    person.phones_count = person_wrapper->phones_count;
    
    for (uint32_t i = 0; i < person.phones_count && i < 10; i++) {
        strncpy(person.phones[i].number, person_wrapper->phones[i].number, sizeof(person.phones[i].number) - 1);
        person.phones[i].type = (demo_Person_PhoneType)person_wrapper->phones[i].type;
    }
    
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
    bool status = pb_encode(&stream, demo_Person_fields, &person);
    if (bytes_written) {
        *bytes_written = stream.bytes_written;
    }
    return status;
}

extern "C" bool nanopb_decode_person(const uint8_t* buffer, size_t size, demo_Person_nanopb* person_wrapper) {
    demo_Person person = demo_Person_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(buffer, size);
    bool status = pb_decode(&stream, demo_Person_fields, &person);
    
    if (status) {
        strncpy(person_wrapper->name, person.name, sizeof(person_wrapper->name) - 1);
        person_wrapper->name[sizeof(person_wrapper->name) - 1] = '\0';
        person_wrapper->id = person.id;
        strncpy(person_wrapper->email, person.email, sizeof(person_wrapper->email) - 1);
        person_wrapper->email[sizeof(person_wrapper->email) - 1] = '\0';
        person_wrapper->phones_count = person.phones_count;
        
        for (uint32_t i = 0; i < person.phones_count && i < 10; i++) {
            strncpy(person_wrapper->phones[i].number, person.phones[i].number, sizeof(person_wrapper->phones[i].number) - 1);
            person_wrapper->phones[i].number[sizeof(person_wrapper->phones[i].number) - 1] = '\0';
            person_wrapper->phones[i].type = person.phones[i].type;
        }
    }
    
    return status;
}

// Aliases for comparison benchmark
extern "C" bool nanopb_encode_msg(const nanopb_Person* person, uint8_t* buffer, size_t buffer_size, size_t* bytes_written) {
    return nanopb_encode_person(person, buffer, buffer_size, bytes_written);
}

extern "C" bool nanopb_decode_msg(const uint8_t* buffer, size_t size, nanopb_Person* person) {
    return nanopb_decode_person(buffer, size, person);
}
