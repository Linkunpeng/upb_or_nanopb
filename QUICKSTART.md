# Quick Start Guide

This guide will help you get started with the upb_or_nanopb comparison project in minutes.

## Prerequisites

Make sure you have the following installed:

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake protobuf-compiler python3-pip git
pip install protobuf grpcio-tools

# macOS
brew install cmake protobuf python3
pip3 install protobuf grpcio-tools
```

## Build in 3 Steps

```bash
# 1. Clone the repository
git clone https://github.com/Linkunpeng/upb_or_nanopb.git
cd upb_or_nanopb

# 2. Build
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)

# 3. Run
./nanopb_demo
```

## What You Get

After building, you'll have:

### Executables
- `nanopb_demo` - Demo application showing nanopb usage
- `nanopb_test` - Unit tests (run with `./nanopb_test` or `ctest`)
- `nanopb_benchmark` - Performance benchmarks

### Try It Out

```bash
# Run the demo
./nanopb_demo

# Run tests
ctest --output-on-failure
# or
./nanopb_test

# Run benchmarks
./nanopb_benchmark

# Check binary size
ls -lh nanopb_demo
strip nanopb_demo
ls -lh nanopb_demo
```

## Expected Output

### Demo Output
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

### Test Output
```
[==========] Running 4 tests from 1 test suite.
[  PASSED  ] 4 tests.
```

### Benchmark Output
```
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_Nanopb_Encode             1092 ns         1092 ns       644589
BM_Nanopb_Decode              876 ns          876 ns       799296
BM_Nanopb_RoundTrip          2030 ns         2030 ns       345261
```

## Project Structure

```
upb_or_nanopb/
├── proto/              # Protocol buffer definitions
│   ├── person.proto    # Message definitions
│   └── person.options  # Nanopb configuration
├── src/
│   └── nanopb_example/ # Example implementation
├── tests/              # Unit tests
└── benchmarks/         # Performance benchmarks
```

## Customizing

### Modify the Proto File

Edit `proto/person.proto` to change the message structure.

### Adjust Buffer Sizes

Edit `proto/person.options` to change maximum sizes:

```
demo.Person.name max_size:64
demo.Person.email max_size:64
demo.Person.PhoneNumber.number max_size:32
demo.Person.phones max_count:10
```

### Rebuild After Changes

```bash
cd build
cmake --build .
```

## Troubleshooting

### protoc not found
```bash
sudo apt-get install -y protobuf-compiler
```

### Python protobuf module not found
```bash
pip install protobuf grpcio-tools
```

### Build fails
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

## Next Steps

- Read [README.md](README.md) for detailed documentation
- Check [RESULTS.md](RESULTS.md) for performance metrics
- Explore the source code in `src/`, `tests/`, and `benchmarks/`
- Modify the proto file and see how it affects size and performance

## Need Help?

- Review the detailed [README.md](README.md)
- Check the [nanopb documentation](https://jpa.kapsi.fi/nanopb/docs/)
- Look at example code in `src/nanopb_example/main.c`

## Future Updates

- UPB implementation coming soon
- Direct performance comparison between UPB and Nanopb
- More advanced examples

Happy coding! 🚀
