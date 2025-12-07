cmake_minimum_required(VERSION 3.15)
project(upb_nanopb_comparison CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch nanopb
FetchContent_Declare(
  nanopb
  GIT_REPOSITORY https://github.com/nanopb/nanopb.git
  GIT_TAG 0.4.8
)

# Fetch Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

# Fetch Google Benchmark
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(nanopb googletest benchmark)

enable_testing()

# Find protoc
find_program(PROTOC protoc)
if(NOT PROTOC)
  message(FATAL_ERROR "protoc not found. Please install protobuf compiler.")
endif()

# Proto files
set(PROTO_FILE ${CMAKE_SOURCE_DIR}/proto/person.proto)
set(PROTO_OPTIONS ${CMAKE_SOURCE_DIR}/proto/person.options)
set(SERVICE_PROTO_FILE ${CMAKE_SOURCE_DIR}/proto/person_service.proto)
set(SERVICE_PROTO_OPTIONS ${CMAKE_SOURCE_DIR}/proto/person_service.options)

# Generate nanopb files
set(NANOPB_OUT_DIR ${CMAKE_BINARY_DIR}/generated/nanopb)
file(MAKE_DIRECTORY ${NANOPB_OUT_DIR})

set(NANOPB_GENERATOR ${nanopb_SOURCE_DIR}/generator/nanopb_generator.py)
set(NANOPB_PLUGIN ${nanopb_SOURCE_DIR}/generator/protoc-gen-nanopb)

add_custom_command(
  OUTPUT ${NANOPB_OUT_DIR}/person.pb.h ${NANOPB_OUT_DIR}/person.pb.c
  COMMAND ${PROTOC} 
    --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN}
    --nanopb_opt=-f${CMAKE_SOURCE_DIR}/proto/person.options
    --nanopb_out=${NANOPB_OUT_DIR}
    -I${CMAKE_SOURCE_DIR}/proto 
    ${PROTO_FILE}
  DEPENDS ${PROTO_FILE} ${PROTO_OPTIONS}
  COMMENT "Generating nanopb files for person.proto"
)

# Generate nanopb files for service
add_custom_command(
  OUTPUT ${NANOPB_OUT_DIR}/person_service.pb.h ${NANOPB_OUT_DIR}/person_service.pb.c
  COMMAND ${PROTOC} 
    --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN}
    --nanopb_opt=-f${CMAKE_SOURCE_DIR}/proto/person_service.options
    --nanopb_out=${NANOPB_OUT_DIR}
    -I${CMAKE_SOURCE_DIR}/proto 
    ${SERVICE_PROTO_FILE}
  DEPENDS ${SERVICE_PROTO_FILE} ${SERVICE_PROTO_OPTIONS} ${PROTO_FILE}
  COMMENT "Generating nanopb files for person_service.proto"
)

# Nanopb Example
add_executable(nanopb_demo
  src/nanopb_example/main.c
  ${NANOPB_OUT_DIR}/person.pb.c
)
target_include_directories(nanopb_demo PRIVATE 
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
)
target_link_libraries(nanopb_demo PRIVATE protobuf-nanopb-static)

# RPC Library
add_library(simple_rpc STATIC
  src/rpc/simple_rpc.c
)
target_include_directories(simple_rpc PUBLIC
  src/rpc
)

# RPC Server
add_executable(rpc_server
  src/rpc/rpc_server.c
  ${NANOPB_OUT_DIR}/person.pb.c
  ${NANOPB_OUT_DIR}/person_service.pb.c
)
target_include_directories(rpc_server PRIVATE 
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
  src/rpc
)
target_link_libraries(rpc_server PRIVATE 
  protobuf-nanopb-static
  simple_rpc
)

# RPC Client
add_executable(rpc_client
  src/rpc/rpc_client.c
  ${NANOPB_OUT_DIR}/person.pb.c
  ${NANOPB_OUT_DIR}/person_service.pb.c
)
target_include_directories(rpc_client PRIVATE 
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
  src/rpc
)
target_link_libraries(rpc_client PRIVATE 
  protobuf-nanopb-static
  simple_rpc
)

# Tests
add_executable(nanopb_test
  tests/nanopb_test.cpp
  ${NANOPB_OUT_DIR}/person.pb.c
)
target_include_directories(nanopb_test PRIVATE 
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
)
target_link_libraries(nanopb_test PRIVATE 
  protobuf-nanopb-static
  gtest
  gtest_main
)

# RPC Test
add_executable(rpc_test
  tests/rpc_test.cpp
  ${NANOPB_OUT_DIR}/person.pb.c
  ${NANOPB_OUT_DIR}/person_service.pb.c
)
target_include_directories(rpc_test PRIVATE 
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
  src/rpc
)
target_link_libraries(rpc_test PRIVATE 
  protobuf-nanopb-static
  simple_rpc
  gtest
  gtest_main
)

# Benchmarks
add_executable(nanopb_benchmark
  benchmarks/nanopb_benchmark.cpp
  ${NANOPB_OUT_DIR}/person.pb.c
)
target_include_directories(nanopb_benchmark PRIVATE 
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
)
target_link_libraries(nanopb_benchmark PRIVATE 
  protobuf-nanopb-static
  benchmark::benchmark
)

add_test(NAME nanopb_test COMMAND nanopb_test)
add_test(NAME rpc_test COMMAND rpc_test)

# Add custom target to show binary sizes
add_custom_target(show_sizes
  COMMAND ${CMAKE_COMMAND} -E echo "Binary size comparison:"
  COMMAND ${CMAKE_COMMAND} -E echo "Nanopb demo:"
  COMMAND ls -lh $<TARGET_FILE:nanopb_demo>
  COMMAND ${CMAKE_COMMAND} -E echo "RPC server:"
  COMMAND ls -lh $<TARGET_FILE:rpc_server>
  COMMAND ${CMAKE_COMMAND} -E echo "RPC client:"
  COMMAND ls -lh $<TARGET_FILE:rpc_client>
  DEPENDS nanopb_demo rpc_server rpc_client
)
