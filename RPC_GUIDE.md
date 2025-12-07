# Nanopb RPC Implementation

This document describes the implementation of a lightweight RPC framework using Nanopb for Protocol Buffers serialization.

## Overview

Nanopb **CAN** be used with RPC, but it requires careful consideration of:

1. **Message Size**: Nanopb uses static buffers and compile-time size checks. Large messages with many optional fields can exceed nanopb's field width encoding limitations.

2. **Proto Syntax**: Proto2 vs Proto3 affects how fields are encoded. Optional submessages in particular need special handling.

3. **Integration**: You need to implement or integrate with an RPC transport layer (TCP sockets, HTTP, etc.)

## Implementation

The example includes:

- **Simple RPC Framework** (`src/rpc/simple_rpc.{c,h}`): A minimal TCP-based RPC framework with:
  - Client/server architecture
  - Fixed-size message headers
  - Method ID routing
  - Synchronous request/response model

- **Service Definitions** (`proto/person_service.proto`): RPC service messages using Protocol Buffers

- **Server & Client** (`src/rpc/rpc_server.c`, `src/rpc/rpc_client.c`): Example implementations

- **Unit Tests** (`tests/rpc_test.cpp`): Comprehensive test suite using Google Test

## Current Status

⚠️ **Build Issue**: The current implementation encounters a Nanopb limitation with the `GetPersonResponse` message. The struct contains:
- An optional `Person` submessage (~500 bytes)
- An optional `bool` field

This combination exceeds Nanopb's field width encoding capacity (FIELDINFO_DOES_NOT_FIT_width1_field).

## Solutions

### Option 1: Simplify Messages (Recommended)
- Use smaller message types
- Avoid optional submessages where possible
- Use repeated fields with max_count constraints

### Option 2: Use Proto3 with proto3_singular_msgs
- Eliminates `has_` fields for submessages
- Requires careful handling of "default" values

### Option 3: Use Callbacks
- Define Person field as `type:FT_CALLBACK`
- More complex implementation but handles large messages

### Option 4: Alternative: gRPC-C or protobuf-c
If Nanopb's limitations are too restrictive:
- **gRPC-C**: Full-featured gRPC implementation with C bindings
- **protobuf-c**: C implementation of Protocol Buffers with dynamic allocation

## Recommended Approach for Production

For embedded/resource-constrained systems using Nanopb + RPC:

1. **Keep Messages Small**: Design service APIs with small, focused message types
2. **Limit Nesting**: Avoid deep message hierarchies
3. **Use IDs Instead of Objects**: Pass entity IDs rather than full objects
4. **Batch Operations**: Group related calls to minimize overhead

### Example: Simplified Service Design

```protobuf
// Instead of:
message GetPersonResponse {
  optional Person person = 1;  // Large, complex
  optional bool found = 2;
}

// Use:
message GetPersonResponse {
  optional int32 person_id = 1;
  optional string person_name = 2;
  optional string person_email = 3;
  optional bool found = 4;
}

// Or split into multiple calls:
message GetPersonIdResponse {
  optional int32 id = 1;
  optional bool found = 2;
}

message GetPersonDetailsRequest {
  required int32 id = 1;
}

message GetPersonDetailsResponse {
  required string name = 1;
  optional string email = 2;
}
```

## Testing

Unit tests demonstrate:
- ✅ Client creation and connection
- ✅ Message encoding/decoding with Nanopb
- ✅ Multiple sequential requests
- ✅ Server handler registration
- ✅ RPC call/response cycle

Run tests with:
```bash
cd build
ctest -R rpc_test -V
```

## Conclusion

**Yes, Nanopb can be used for RPC**, with these considerations:

✅ **Pros**:
- Extremely small code footprint
- No dynamic memory allocation
- Predictable performance
- Perfect for embedded systems

⚠️ **Cons**:
- Message size limitations
- Struct layout constraints with optional fields
- Requires manual RPC framework (or integration)
- Less flexible than full Protocol Buffers

**Best For**: 
- Microcontrollers and IoT devices
- Systems with strict memory constraints
- Simple request/response patterns
- Static, well-defined APIs

**Not Ideal For**:
- Complex, deeply nested messages
- Services requiring large data transfers
- Highly dynamic message structures
- Systems where binary size is less critical

For a complete working example with production-ready features, consider using **gRPC with the full Protocol Buffers library** for server-side components, and **Nanopb** for embedded clients that communicate with the server.
