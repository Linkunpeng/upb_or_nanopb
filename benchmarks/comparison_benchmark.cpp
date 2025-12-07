#include <benchmark/benchmark.h>
#include <cstring>
#include <string>

// Forward declare to avoid including both headers which conflict
namespace demo {
    class Person;
}

extern "C" {
    struct demo_Person;
}

// We'll use wrapper functions to avoid header conflicts

// C++ Protobuf wrapper functions
void cpp_pb_create_person(void** person_ptr);
void cpp_pb_set_data(void* person_ptr);
std::string cpp_pb_serialize(void* person_ptr);
void* cpp_pb_deserialize(const std::string& data);
void cpp_pb_delete(void* person_ptr);

// Nanopb wrapper functions  
void nanopb_create_person(void** person_ptr);
void nanopb_set_data(void* person_ptr);
size_t nanopb_serialize(void* person_ptr, uint8_t* buffer, size_t size);
void nanopb_deserialize(const uint8_t* buffer, size_t size, void* person_ptr);

// For simplicity, let's just use the individual benchmarks
// Users can run both nanopb_benchmark and cpp_pb_benchmark separately

#include "person.pb.h"

// C++ Protobuf Encode
static void BM_CppPb_Encode(benchmark::State& state) {
    for (auto _ : state) {
        demo::Person person;
        person.set_name("John Doe");
        person.set_id(12345);
        person.set_email("john.doe@example.com");
        
        auto* phone1 = person.add_phones();
        phone1->set_number("555-1234");
        phone1->set_type(demo::Person::MOBILE);
        
        auto* phone2 = person.add_phones();
        phone2->set_number("555-5678");
        phone2->set_type(demo::Person::WORK);
        
        std::string encoded;
        person.SerializeToString(&encoded);
        benchmark::DoNotOptimize(encoded);
    }
}
BENCHMARK(BM_CppPb_Encode);

// C++ Protobuf Decode
static void BM_CppPb_Decode(benchmark::State& state) {
    // Pre-encode
    demo::Person person;
    person.set_name("John Doe");
    person.set_id(12345);
    person.set_email("john.doe@example.com");
    
    auto* phone1 = person.add_phones();
    phone1->set_number("555-1234");
    phone1->set_type(demo::Person::MOBILE);
    
    std::string encoded;
    person.SerializeToString(&encoded);
    
    for (auto _ : state) {
        demo::Person decoded;
        decoded.ParseFromString(encoded);
        benchmark::DoNotOptimize(decoded);
    }
}
BENCHMARK(BM_CppPb_Decode);

// C++ Protobuf Round Trip
static void BM_CppPb_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        demo::Person person;
        person.set_name("John Doe");
        person.set_id(12345);
        person.set_email("john.doe@example.com");
        
        auto* phone1 = person.add_phones();
        phone1->set_number("555-1234");
        phone1->set_type(demo::Person::MOBILE);
        
        std::string encoded;
        person.SerializeToString(&encoded);
        
        demo::Person decoded;
        decoded.ParseFromString(encoded);
        benchmark::DoNotOptimize(decoded);
    }
}
BENCHMARK(BM_CppPb_RoundTrip);

BENCHMARK_MAIN();
