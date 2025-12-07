#include <benchmark/benchmark.h>
#include "person.pb.h"

// C++ Protobuf Benchmark: Encode
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

// C++ Protobuf Benchmark: Decode
static void BM_CppPb_Decode(benchmark::State& state) {
    // Pre-encode a message
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

// C++ Protobuf Benchmark: Round Trip
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

// Benchmark with small messages
static void BM_CppPb_Encode_Small(benchmark::State& state) {
    for (auto _ : state) {
        demo::Person person;
        person.set_name("John");
        person.set_id(42);
        
        std::string encoded;
        person.SerializeToString(&encoded);
        benchmark::DoNotOptimize(encoded);
    }
}
BENCHMARK(BM_CppPb_Encode_Small);

// Benchmark with large messages
static void BM_CppPb_Encode_Large(benchmark::State& state) {
    for (auto _ : state) {
        demo::Person person;
        person.set_name("John Doe with a very long name for testing");
        person.set_id(12345);
        person.set_email("john.doe.very.long.email@example.com");
        
        for (int i = 0; i < 10; i++) {
            auto* phone = person.add_phones();
            phone->set_number("555-" + std::to_string(i));
            phone->set_type(static_cast<demo::Person::PhoneType>(i % 3));
        }
        
        std::string encoded;
        person.SerializeToString(&encoded);
        benchmark::DoNotOptimize(encoded);
    }
}
BENCHMARK(BM_CppPb_Encode_Large);

BENCHMARK_MAIN();
