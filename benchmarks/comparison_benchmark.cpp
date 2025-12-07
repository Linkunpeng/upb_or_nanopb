#include <benchmark/benchmark.h>
#include <cstring>

// UPB includes
extern "C" {
#include "person.upb.h"
#include "upb/mem/arena.h"
#include "upb/encode.h"
#include "upb/decode.h"
}

// Nanopb includes - use separate namespace to avoid conflicts
extern "C" {
// Forward declarations for nanopb
typedef struct pb_istream_s pb_istream_t;
typedef struct pb_ostream_s pb_ostream_t;
typedef struct pb_msgdesc_s pb_msgdesc_t;

// Simplified nanopb person struct for benchmarking
typedef struct {
    char name[64];
    int32_t id;
    char email[64];
    uint32_t phones_count;
    struct {
        char number[32];
        int type;
    } phones[10];
} nanopb_Person;

// Nanopb encode/decode wrappers
bool nanopb_encode_msg(const nanopb_Person* person, uint8_t* buffer, size_t buffer_size, size_t* bytes_written);
bool nanopb_decode_msg(const uint8_t* buffer, size_t size, nanopb_Person* person);
}

// UPB Encode
static void BM_UPB_Encode(benchmark::State& state) {
    for (auto _ : state) {
        upb_Arena* arena = upb_Arena_New();
        
        demo_Person* person = demo_Person_new(arena);
        demo_Person_set_name(person, upb_StringView_FromString("John Doe"));
        demo_Person_set_id(person, 12345);
        demo_Person_set_email(person, upb_StringView_FromString("john.doe@example.com"));
        
        demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, arena);
        demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("555-1234"));
        demo_Person_PhoneNumber_set_type(phone1, demo_Person_PhoneType_MOBILE);
        
        demo_Person_PhoneNumber* phone2 = demo_Person_add_phones(person, arena);
        demo_Person_PhoneNumber_set_number(phone2, upb_StringView_FromString("555-5678"));
        demo_Person_PhoneNumber_set_type(phone2, demo_Person_PhoneType_WORK);
        
        size_t size;
        char* encoded = demo_Person_serialize(person, arena, &size);
        benchmark::DoNotOptimize(encoded);
        
        upb_Arena_Free(arena);
    }
}
BENCHMARK(BM_UPB_Encode);

// Nanopb Encode
static void BM_Nanopb_Encode(benchmark::State& state) {
    for (auto _ : state) {
        nanopb_Person person = {};
        strncpy(person.name, "John Doe", sizeof(person.name) - 1);
        person.name[sizeof(person.name) - 1] = '\0';
        person.id = 12345;
        strncpy(person.email, "john.doe@example.com", sizeof(person.email) - 1);
        person.email[sizeof(person.email) - 1] = '\0';
        
        person.phones_count = 2;
        strncpy(person.phones[0].number, "555-1234", sizeof(person.phones[0].number) - 1);
        person.phones[0].number[sizeof(person.phones[0].number) - 1] = '\0';
        person.phones[0].type = 0;  // MOBILE
        strncpy(person.phones[1].number, "555-5678", sizeof(person.phones[1].number) - 1);
        person.phones[1].number[sizeof(person.phones[1].number) - 1] = '\0';
        person.phones[1].type = 2;  // WORK
        
        uint8_t buffer[256];
        size_t bytes_written;
        nanopb_encode_msg(&person, buffer, sizeof(buffer), &bytes_written);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_Nanopb_Encode);

// UPB Round Trip
static void BM_UPB_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        upb_Arena* arena = upb_Arena_New();
        
        demo_Person* person = demo_Person_new(arena);
        demo_Person_set_name(person, upb_StringView_FromString("John Doe"));
        demo_Person_set_id(person, 12345);
        demo_Person_set_email(person, upb_StringView_FromString("john.doe@example.com"));
        
        demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, arena);
        demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("555-1234"));
        demo_Person_PhoneNumber_set_type(phone1, demo_Person_PhoneType_MOBILE);
        
        size_t size;
        char* encoded = demo_Person_serialize(person, arena, &size);
        demo_Person* decoded = demo_Person_parse(encoded, size, arena);
        benchmark::DoNotOptimize(decoded);
        
        upb_Arena_Free(arena);
    }
}
BENCHMARK(BM_UPB_RoundTrip);

// Nanopb Round Trip
static void BM_Nanopb_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        nanopb_Person person = {};
        strncpy(person.name, "John Doe", sizeof(person.name) - 1);
        person.name[sizeof(person.name) - 1] = '\0';
        person.id = 12345;
        strncpy(person.email, "john.doe@example.com", sizeof(person.email) - 1);
        person.email[sizeof(person.email) - 1] = '\0';
        
        person.phones_count = 2;
        strncpy(person.phones[0].number, "555-1234", sizeof(person.phones[0].number) - 1);
        person.phones[0].number[sizeof(person.phones[0].number) - 1] = '\0';
        person.phones[0].type = 0;  // MOBILE
        strncpy(person.phones[1].number, "555-5678", sizeof(person.phones[1].number) - 1);
        person.phones[1].number[sizeof(person.phones[1].number) - 1] = '\0';
        person.phones[1].type = 2;  // WORK
        
        uint8_t buffer[256];
        size_t bytes_written;
        nanopb_encode_msg(&person, buffer, sizeof(buffer), &bytes_written);
        
        nanopb_Person decoded = {};
        nanopb_decode_msg(buffer, bytes_written, &decoded);
        benchmark::DoNotOptimize(decoded);
    }
}
BENCHMARK(BM_Nanopb_RoundTrip);

BENCHMARK_MAIN();
