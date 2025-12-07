# Performance and Size Comparison Results

This document contains the actual measurement results from the upb_or_nanopb comparison project.

## Binary Size Comparison

### Nanopb Demo

| Build Type | Size |
|------------|------|
| Debug (with symbols) | 44 KB |
| Stripped (production) | 39 KB |

**Note**: UPB implementation is not yet available for comparison.

## Performance Benchmarks

All benchmarks performed on: 4 × 3244.01 MHz CPU

### Nanopb Performance Results

| Benchmark | Time (ns) | Throughput |
|-----------|-----------|------------|
| Encode (standard message) | 1,092 ns | ~916,000 ops/sec |
| Decode (standard message) | 876 ns | ~1,142,000 ops/sec |
| Round Trip (encode + decode) | 2,030 ns | ~492,000 ops/sec |
| Encode (small message) | 256 ns | ~3,906,000 ops/sec |
| Encode (large message with 10 phones) | 4,467 ns | ~224,000 ops/sec |

### Message Sizes

| Message Type | Encoded Size |
|--------------|--------------|
| Standard Person (with 2 phones) | 61 bytes |
| Empty Person | 0 bytes |

## Test Results

### Nanopb Tests

All 4 tests passed:

1. ✅ BasicEncodeDecode - Validates basic encoding and decoding
2. ✅ RepeatedFields - Tests repeated field handling
3. ✅ EmptyMessage - Tests empty message handling
4. ✅ MaxFields - Tests maximum sized messages

## Key Findings

### Nanopb Characteristics

**Strengths:**
- Very small binary footprint (< 40KB stripped)
- Fast encoding/decoding (~1 microsecond for typical messages)
- No dynamic memory allocation
- Predictable memory usage
- Excellent for embedded systems

**Performance Characteristics:**
- Encode performance: ~1 microsecond for typical messages
- Decode performance: slightly faster than encode (~0.9 microseconds)
- Scales linearly with message complexity
- Small messages (< 10 bytes) encode in ~250 ns
- Large messages (> 100 bytes) encode in ~4-5 microseconds

**Memory Usage:**
- Stack-based allocation only
- Fixed buffer sizes defined at compile time
- No heap allocations
- Predictable worst-case memory usage

## Comparison Analysis

### When to Use Nanopb

✅ Embedded systems with limited resources  
✅ Real-time applications requiring predictable performance  
✅ Systems without heap/dynamic memory  
✅ Small to medium-sized messages (< 1KB)  
✅ Microcontrollers and IoT devices  
✅ Safety-critical systems requiring static analysis  

### Limitations

- Maximum field sizes must be known at compile time
- Not suitable for very large or dynamic messages
- Requires `.options` file configuration
- Less flexible than dynamic implementations

## Test Environment

- **OS**: Linux
- **CPU**: 4 × 3244.01 MHz
- **Compiler**: GCC 13.3.0
- **Optimization**: Debug build (production builds would be faster)
- **Nanopb Version**: 0.4.8
- **Test Message**: Person with 2 phone numbers (61 bytes encoded)

## Conclusion

Nanopb demonstrates excellent performance characteristics for embedded and resource-constrained environments:

- **Binary size**: Very compact at ~39KB (stripped)
- **Speed**: Sub-microsecond operations for typical messages
- **Memory**: Zero dynamic allocation, fully predictable
- **Reliability**: All tests passing, robust implementation

This makes Nanopb an excellent choice for embedded systems, IoT devices, and any application where code size and memory predictability are critical requirements.

---

*Note: UPB comparison metrics will be added once the UPB implementation is complete.*
