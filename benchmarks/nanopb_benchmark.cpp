#include <benchmark/benchmark.h>
#include <cstring>

// Nanopb includes
extern "C" {
#include "person.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
}

// Nanopb Benchmark: Encode
static void BM_Nanopb_Encode(benchmark::State& state) {
    for (auto _ : state) {
        demo_Person person = demo_Person_init_zero;
        strncpy(person.name, "John Doe", sizeof(person.name) - 1);
        person.name[sizeof(person.name) - 1] = '\0';
        person.id = 12345;
        strncpy(person.email, "john.doe@example.com", sizeof(person.email) - 1);
        person.email[sizeof(person.email) - 1] = '\0';
        
        person.phones_count = 2;
        strncpy(person.phones[0].number, "555-1234", sizeof(person.phones[0].number) - 1);
        person.phones[0].number[sizeof(person.phones[0].number) - 1] = '\0';
        person.phones[0].type = demo_Person_PhoneType_MOBILE;
        strncpy(person.phones[1].number, "555-5678", sizeof(person.phones[1].number) - 1);
        person.phones[1].number[sizeof(person.phones[1].number) - 1] = '\0';
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
    strncpy(person.name, "John Doe", sizeof(person.name) - 1);
    person.name[sizeof(person.name) - 1] = '\0';
    person.id = 12345;
    strncpy(person.email, "john.doe@example.com", sizeof(person.email) - 1);
    person.email[sizeof(person.email) - 1] = '\0';
    
    person.phones_count = 2;
    strncpy(person.phones[0].number, "555-1234", sizeof(person.phones[0].number) - 1);
    person.phones[0].number[sizeof(person.phones[0].number) - 1] = '\0';
    person.phones[0].type = demo_Person_PhoneType_MOBILE;
    strncpy(person.phones[1].number, "555-5678", sizeof(person.phones[1].number) - 1);
    person.phones[1].number[sizeof(person.phones[1].number) - 1] = '\0';
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
        strncpy(person.name, "John Doe", sizeof(person.name) - 1);
        person.name[sizeof(person.name) - 1] = '\0';
        person.id = 12345;
        strncpy(person.email, "john.doe@example.com", sizeof(person.email) - 1);
        person.email[sizeof(person.email) - 1] = '\0';
        
        person.phones_count = 2;
        strncpy(person.phones[0].number, "555-1234", sizeof(person.phones[0].number) - 1);
        person.phones[0].number[sizeof(person.phones[0].number) - 1] = '\0';
        person.phones[0].type = demo_Person_PhoneType_MOBILE;
        strncpy(person.phones[1].number, "555-5678", sizeof(person.phones[1].number) - 1);
        person.phones[1].number[sizeof(person.phones[1].number) - 1] = '\0';
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

// Benchmark with small messages (no phone numbers)
static void BM_Nanopb_Encode_Small(benchmark::State& state) {
    for (auto _ : state) {
        demo_Person person = demo_Person_init_zero;
        strncpy(person.name, "John", sizeof(person.name) - 1);
        person.name[sizeof(person.name) - 1] = '\0';
        person.id = 42;
        
        uint8_t buffer[256];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        pb_encode(&stream, demo_Person_fields, &person);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_Nanopb_Encode_Small);

// Benchmark with large messages (maximum phones)
static void BM_Nanopb_Encode_Large(benchmark::State& state) {
    for (auto _ : state) {
        demo_Person person = demo_Person_init_zero;
        strncpy(person.name, "John Doe with a very long name", sizeof(person.name) - 1);
        person.name[sizeof(person.name) - 1] = '\0';
        person.id = 12345;
        strncpy(person.email, "john.doe.very.long.email@example.com", sizeof(person.email) - 1);
        person.email[sizeof(person.email) - 1] = '\0';
        
        person.phones_count = 10;  // Max count
        for (size_t i = 0; i < person.phones_count; i++) {
            snprintf(person.phones[i].number, sizeof(person.phones[i].number), "555-%04zu", i);
            person.phones[i].type = (demo_Person_PhoneType)(i % 3);
        }
        
        uint8_t buffer[1024];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        pb_encode(&stream, demo_Person_fields, &person);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_Nanopb_Encode_Large);

BENCHMARK_MAIN();
