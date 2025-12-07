# UPB vs Nanopb Comparison

This project provides a comprehensive comparison between two popular Protocol Buffers implementations for embedded and resource-constrained systems: **UPB** and **Nanopb**.

**Note**: Currently, this project demonstrates **Nanopb** implementation with complete working examples, tests, and benchmarks. UPB integration is planned for future updates.

## Overview

The project includes:
- A common `.proto` file (person.proto) used for demonstrations
- Example applications demonstrating Nanopb usage
- Benchmark tests to measure performance (encoding, decoding, and round-trip)
- Unit tests with Google Test to verify functionality
- Binary size analysis of the compiled executables

## Features

- **Nanopb Example**: Shows stack-based allocation with static buffers
- **Benchmarks**: Measures encoding, decoding, and round-trip performance
- **Tests**: Validates correctness including edge cases
- **Size Analysis**: Shows binary sizes of the compiled executables

## Prerequisites

- CMake 3.15 or higher
- C/C++ compiler (GCC, Clang, or MSVC)
- Protocol Buffers compiler (protoc)
- Python 3 with protobuf package
- Git

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y protobuf-compiler python3-pip
pip install protobuf grpcio-tools
```

**macOS:**
```bash
brew install protobuf python3
pip3 install protobuf grpcio-tools
```

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Optional: Build with specific number of jobs
cmake --build . -j$(nproc)
```

## Running Examples

After building, you can run the demo program:

```bash
# Run Nanopb demo
./nanopb_demo
```

Example output:
```
Nanopb Demo
===========

Encoded message size: 61 bytes

Decoded Person:
  Name: John Doe
  ID: 12345
  Email: john.doe@example.com
  Phones (2):
    [0] 555-1234 (MOBILE)
    [1] 555-5678 (WORK)

Nanopb demo completed successfully!
```

## Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Or run the test executable directly
./nanopb_test
```

Test output:
```
[==========] Running 4 tests from 1 test suite.
[----------] 4 tests from NanopbTest
[ RUN      ] NanopbTest.BasicEncodeDecode
[       OK ] NanopbTest.BasicEncodeDecode (0 ms)
[ RUN      ] NanopbTest.RepeatedFields
[       OK ] NanopbTest.RepeatedFields (0 ms)
[ RUN      ] NanopbTest.EmptyMessage
[       OK ] NanopbTest.EmptyMessage (0 ms)
[ RUN      ] NanopbTest.MaxFields
[       OK ] NanopbTest.MaxFields (0 ms)
[==========] 4 tests from 1 test suite ran.
[  PASSED  ] 4 tests.
```

## Running Benchmarks

```bash
# Run all benchmarks
./nanopb_benchmark

# Run with longer time for more accurate results
./nanopb_benchmark --benchmark_min_time=1.0s

# Get more detailed output
./nanopb_benchmark --benchmark_repetitions=10
```

Example benchmark results:
```
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_Nanopb_Encode             1092 ns         1092 ns       644589
BM_Nanopb_Decode              876 ns          876 ns       799296
BM_Nanopb_RoundTrip          2030 ns         2030 ns       345261
BM_Nanopb_Encode_Small        256 ns          256 ns      2736318
BM_Nanopb_Encode_Large       4467 ns         4466 ns       157025
```

## Binary Size Analysis

To check the binary size:

```bash
# Before stripping debug symbols
ls -lh nanopb_demo

# After stripping (production size)
strip nanopb_demo
ls -lh nanopb_demo
```

Example sizes:
- Debug build: ~44 KB
- Stripped build: ~39 KB

## Project Structure

```
.
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── proto/
│   ├── person.proto            # Protobuf definition
│   └── person.options          # Nanopb-specific options
├── src/
│   └── nanopb_example/
│       └── main.c              # Nanopb demonstration
├── benchmarks/
│   └── nanopb_benchmark.cpp    # Performance benchmarks
└── tests/
    └── nanopb_test.cpp         # Unit tests
```

## Nanopb Configuration

The `.options` file configures Nanopb code generation:

```
demo.Person.name max_size:64
demo.Person.email max_size:64
demo.Person.PhoneNumber.number max_size:32
demo.Person.phones max_count:10
demo.AddressBook.people max_count:100
```

These options ensure:
- Fixed-size string buffers (no dynamic allocation)
- Bounded arrays for repeated fields
- Predictable memory usage
- Suitable for embedded systems

## Key Characteristics of Nanopb

### Nanopb
- **Memory Management**: Stack-based with static buffers
- **Memory Model**: Static/stack allocation, no dynamic memory
- **Code Size**: Very small binary (< 50KB stripped)
- **Performance**: ~1 microsecond for simple encode/decode operations
- **Use Case**: Embedded systems, resource-constrained devices, real-time systems

### Advantages
- Minimal code footprint
- No dynamic memory allocation
- Predictable memory usage
- Fast encode/decode for simple messages
- Perfect for microcontrollers

### Limitations
- Fixed buffer sizes must be known at compile time
- Less flexible than dynamic implementations
- Requires `.options` file for configuration

## Dependencies

The project automatically fetches the following dependencies via CMake's FetchContent:
- [Nanopb](https://github.com/nanopb/nanopb) - Plain C protobuf implementation (v0.4.8)
- [Google Test](https://github.com/google/googletest) - Testing framework (v1.14.0)
- [Google Benchmark](https://github.com/google/benchmark) - Benchmarking framework (v1.8.3)

## Future Work

- Add UPB implementation examples
- Direct comparison benchmarks between UPB and Nanopb
- Memory usage profiling
- Code size comparison with identical functionality

## License

This project is for educational and comparison purposes. Please refer to individual library licenses:
- Nanopb: Zlib license
- Google Test: BSD-3-Clause license
- Google Benchmark: Apache License 2.0
