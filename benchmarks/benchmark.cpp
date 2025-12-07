#include <benchmark/benchmark.h>
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

// UPB Benchmark: Encode
static void BM_UPB_Encode(benchmark::State& state) {
    for (auto _ : state) {
        upb_Arena* arena = upb_Arena_New();
        
        demo_Person* person = demo_Person_new(arena);
        demo_Person_set_name(person, upb_StringView_FromString("John Doe"));
        demo_Person_set_id(person, 12345);
        demo_Person_set_email(person, upb_StringView_FromString("john.doe@example.com"));
        
        demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, arena);
        demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("555-1234"));
        demo_Person_PhoneNumber_set_type(phone1, demo_Person_MOBILE);
        
        demo_Person_PhoneNumber* phone2 = demo_Person_add_phones(person, arena);
        demo_Person_PhoneNumber_set_number(phone2, upb_StringView_FromString("555-5678"));
        demo_Person_PhoneNumber_set_type(phone2, demo_Person_WORK);
        
        size_t size;
        char* encoded = demo_Person_serialize(person, arena, &size);
        benchmark::DoNotOptimize(encoded);
        
        upb_Arena_Free(arena);
    }
}
BENCHMARK(BM_UPB_Encode);

// UPB Benchmark: Decode
static void BM_UPB_Decode(benchmark::State& state) {
    // Pre-encode a message
    upb_Arena* setup_arena = upb_Arena_New();
    demo_Person* person = demo_Person_new(setup_arena);
    demo_Person_set_name(person, upb_StringView_FromString("John Doe"));
    demo_Person_set_id(person, 12345);
    demo_Person_set_email(person, upb_StringView_FromString("john.doe@example.com"));
    
    demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, setup_arena);
    demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("555-1234"));
    demo_Person_PhoneNumber_set_type(phone1, demo_Person_MOBILE);
    
    size_t encoded_size;
    char* encoded_data = demo_Person_serialize(person, setup_arena, &encoded_size);
    
    for (auto _ : state) {
        upb_Arena* arena = upb_Arena_New();
        demo_Person* decoded = demo_Person_parse(encoded_data, encoded_size, arena);
        benchmark::DoNotOptimize(decoded);
        upb_Arena_Free(arena);
    }
    
    upb_Arena_Free(setup_arena);
}
BENCHMARK(BM_UPB_Decode);

// UPB Benchmark: Encode + Decode
static void BM_UPB_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        upb_Arena* arena = upb_Arena_New();
        
        // Encode
        demo_Person* person = demo_Person_new(arena);
        demo_Person_set_name(person, upb_StringView_FromString("John Doe"));
        demo_Person_set_id(person, 12345);
        demo_Person_set_email(person, upb_StringView_FromString("john.doe@example.com"));
        
        demo_Person_PhoneNumber* phone1 = demo_Person_add_phones(person, arena);
        demo_Person_PhoneNumber_set_number(phone1, upb_StringView_FromString("555-1234"));
        demo_Person_PhoneNumber_set_type(phone1, demo_Person_MOBILE);
        
        size_t size;
        char* encoded = demo_Person_serialize(person, arena, &size);
        
        // Decode
        demo_Person* decoded = demo_Person_parse(encoded, size, arena);
        benchmark::DoNotOptimize(decoded);
        
        upb_Arena_Free(arena);
    }
}
BENCHMARK(BM_UPB_RoundTrip);

// Nanopb Benchmark: Encode
static void BM_Nanopb_Encode(benchmark::State& state) {
    for (auto _ : state) {
        demo_Person person = demo_Person_init_zero;
        strcpy(person.name, "John Doe");
        person.id = 12345;
        strcpy(person.email, "john.doe@example.com");
        
        person.phones_count = 2;
        strcpy(person.phones[0].number, "555-1234");
        person.phones[0].type = demo_Person_PhoneType_MOBILE;
        strcpy(person.phones[1].number, "555-5678");
        person.phones[1].type = demo_Person_PhoneType_WORK;
        
        uint8_t buffer[256];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        pb_encode(&stream, demo_Person_fields, &person);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_Nanopb_Encode);

// Nanopb Benchmark: Decode
static void BM_Nanopb_Decode(benchmark::State& state) {
    // Pre-encode a message
    demo_Person person = demo_Person_init_zero;
    strcpy(person.name, "John Doe");
    person.id = 12345;
    strcpy(person.email, "john.doe@example.com");
    
    person.phones_count = 2;
    strcpy(person.phones[0].number, "555-1234");
    person.phones[0].type = demo_Person_PhoneType_MOBILE;
    strcpy(person.phones[1].number, "555-5678");
    person.phones[1].type = demo_Person_PhoneType_WORK;
    
    uint8_t buffer[256];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&ostream, demo_Person_fields, &person);
    size_t encoded_size = ostream.bytes_written;
    
    for (auto _ : state) {
        demo_Person decoded_person = demo_Person_init_zero;
        pb_istream_t istream = pb_istream_from_buffer(buffer, encoded_size);
        pb_decode(&istream, demo_Person_fields, &decoded_person);
        benchmark::DoNotOptimize(decoded_person);
    }
}
BENCHMARK(BM_Nanopb_Decode);

// Nanopb Benchmark: Encode + Decode
static void BM_Nanopb_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        // Encode
        demo_Person person = demo_Person_init_zero;
        strcpy(person.name, "John Doe");
        person.id = 12345;
        strcpy(person.email, "john.doe@example.com");
        
        person.phones_count = 2;
        strcpy(person.phones[0].number, "555-1234");
        person.phones[0].type = demo_Person_PhoneType_MOBILE;
        strcpy(person.phones[1].number, "555-5678");
        person.phones[1].type = demo_Person_PhoneType_WORK;
        
        uint8_t buffer[256];
        pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        pb_encode(&ostream, demo_Person_fields, &person);
        
        // Decode
        demo_Person decoded_person = demo_Person_init_zero;
        pb_istream_t istream = pb_istream_from_buffer(buffer, ostream.bytes_written);
        pb_decode(&istream, demo_Person_fields, &decoded_person);
        benchmark::DoNotOptimize(decoded_person);
    }
}
BENCHMARK(BM_Nanopb_RoundTrip);

BENCHMARK_MAIN();
