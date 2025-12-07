# UPB vs Nanopb Comparison

This project provides a comprehensive comparison between two popular Protocol Buffers implementations for embedded and resource-constrained systems: **UPB** and **Nanopb**.

## Overview

The project includes:
- A common `.proto` file (person.proto) used by both implementations
- Example applications demonstrating UPB and Nanopb usage
- Benchmark tests to measure performance (encoding, decoding, and round-trip)
- Unit tests with Google Test to verify functionality and cross-compatibility
- Binary size comparison of the compiled executables

## Features

- **UPB Example**: Demonstrates arena-based memory management and protobuf operations
- **Nanopb Example**: Shows stack-based allocation with static buffers
- **Benchmarks**: Measures encoding, decoding, and round-trip performance
- **Tests**: Validates correctness and cross-compatibility between implementations
- **Size Analysis**: Compares binary sizes of the compiled executables

## Prerequisites

- CMake 3.15 or higher
- C/C++ compiler (GCC, Clang, or MSVC)
- Protocol Buffers compiler (protoc)
- Git

### Installing protoc

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y protobuf-compiler
```

**macOS:**
```bash
brew install protobuf
```

**From source:**
```bash
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
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

After building, you can run the demo programs:

```bash
# Run UPB demo
./upb_demo

# Run Nanopb demo
./nanopb_demo
```

## Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Or run the test executable directly
./comparison_test
```

## Running Benchmarks

```bash
# Run all benchmarks
./comparison_benchmark

# Run specific benchmarks
./comparison_benchmark --benchmark_filter=UPB
./comparison_benchmark --benchmark_filter=Nanopb

# Get more detailed output
./comparison_benchmark --benchmark_repetitions=10
```

## Binary Size Comparison

To compare the binary sizes of the two implementations:

```bash
cmake --build . --target show_sizes
```

Or manually:

```bash
ls -lh upb_demo nanopb_demo
```

## Project Structure

```
.
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── proto/
│   ├── person.proto            # Shared protobuf definition
│   └── person.options          # Nanopb-specific options
├── src/
│   ├── upb_example/
│   │   └── main.c             # UPB demonstration
│   └── nanopb_example/
│       └── main.c             # Nanopb demonstration
├── benchmarks/
│   └── benchmark.cpp          # Performance benchmarks
└── tests/
    └── test.cpp               # Unit and compatibility tests
```

## Key Differences

### UPB
- **Memory Management**: Arena-based (pool allocator)
- **Memory Model**: Heap allocation
- **Code Size**: Larger binary
- **Performance**: Generally faster for complex messages
- **Use Case**: Systems with more resources, performance-critical applications

### Nanopb
- **Memory Management**: Stack-based with static buffers
- **Memory Model**: Static/stack allocation
- **Code Size**: Smaller binary
- **Performance**: Efficient for simple messages
- **Use Case**: Embedded systems, resource-constrained devices

## Dependencies

The project automatically fetches the following dependencies via CMake's FetchContent:
- [UPB](https://github.com/protocolbuffers/upb) - Google's lightweight protobuf implementation
- [Nanopb](https://github.com/nanopb/nanopb) - Plain C protobuf implementation
- [Google Test](https://github.com/google/googletest) - Testing framework
- [Google Benchmark](https://github.com/google/benchmark) - Benchmarking framework

## License

This project is for educational and comparison purposes. Please refer to individual library licenses:
- UPB: BSD-style license
- Nanopb: Zlib license
- Google Test: BSD-3-Clause license
- Google Benchmark: Apache License 2.0
