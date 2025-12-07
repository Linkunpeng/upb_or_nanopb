cmake_minimum_required(VERSION 3.15)
project(upb_nanopb_comparison CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch upb (lightweight protobuf implementation)
FetchContent_Declare(
  upb
  GIT_REPOSITORY https://github.com/protocolbuffers/upb.git
  GIT_TAG main
)
set(UPB_BUILD_TESTS OFF CACHE BOOL "" FORCE)

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

FetchContent_MakeAvailable(upb nanopb googletest benchmark)

enable_testing()

# Find protoc
find_program(PROTOC protoc)
if(NOT PROTOC)
  message(FATAL_ERROR "protoc not found. Please install protobuf compiler.")
endif()

# Proto file
set(PROTO_FILE ${CMAKE_SOURCE_DIR}/proto/person.proto)
set(PROTO_OPTIONS ${CMAKE_SOURCE_DIR}/proto/person.options)

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
  COMMENT "Generating nanopb files"
)

# Generate upb files  
set(UPB_OUT_DIR ${CMAKE_BINARY_DIR}/generated/upb)
file(MAKE_DIRECTORY ${UPB_OUT_DIR})

# UPB uses protoc plugins from the upb project
# We'll use upbc (upb compiler) which is built as part of upb
# For now, we'll use a simpler approach with manual file generation
add_custom_command(
  OUTPUT ${UPB_OUT_DIR}/person.upb.h ${UPB_OUT_DIR}/person.upb.c
  COMMAND ${CMAKE_COMMAND} -E echo "Generating UPB files..."
  COMMAND ${PROTOC}
    --plugin=protoc-gen-upb=$<TARGET_FILE:protoc-gen-upb>
    --upb_out=${UPB_OUT_DIR}
    -I${CMAKE_SOURCE_DIR}/proto
    ${PROTO_FILE}
  DEPENDS ${PROTO_FILE} protoc-gen-upb
  COMMENT "Generating upb files"
)

# Custom target to track upb code generation
add_custom_target(upb_generator ALL
  DEPENDS ${UPB_OUT_DIR}/person.upb.h ${UPB_OUT_DIR}/person.upb.c
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

# UPB Example (for comparison with nanopb)
add_executable(upb_demo
  src/upb_example/main.c
  ${UPB_OUT_DIR}/person.upb.c
)
target_include_directories(upb_demo PRIVATE 
  ${UPB_OUT_DIR}
  ${upb_SOURCE_DIR}
)
target_link_libraries(upb_demo PRIVATE 
  upb
  port
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

add_executable(upb_test
  tests/upb_test.cpp
  ${UPB_OUT_DIR}/person.upb.c
)
target_include_directories(upb_test PRIVATE 
  ${UPB_OUT_DIR}
  ${upb_SOURCE_DIR}
)
target_link_libraries(upb_test PRIVATE 
  upb
  port
  gtest
  gtest_main
)

# Comparison test
add_executable(comparison_test
  tests/comparison_test.cpp
  tests/nanopb_wrapper.c
  ${NANOPB_OUT_DIR}/person.pb.c
  ${UPB_OUT_DIR}/person.upb.c
)
target_include_directories(comparison_test PRIVATE 
  ${NANOPB_OUT_DIR}
  ${UPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
  ${upb_SOURCE_DIR}
)
target_link_libraries(comparison_test PRIVATE 
  protobuf-nanopb-static
  upb
  port
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

add_executable(upb_benchmark
  benchmarks/upb_benchmark.cpp
  ${UPB_OUT_DIR}/person.upb.c
)
target_include_directories(upb_benchmark PRIVATE 
  ${UPB_OUT_DIR}
  ${upb_SOURCE_DIR}
)
target_link_libraries(upb_benchmark PRIVATE 
  upb
  port
  benchmark::benchmark
)

add_executable(comparison_benchmark
  benchmarks/comparison_benchmark.cpp
  tests/nanopb_wrapper.c
  ${NANOPB_OUT_DIR}/person.pb.c
  ${UPB_OUT_DIR}/person.upb.c
)
target_include_directories(comparison_benchmark PRIVATE 
  ${NANOPB_OUT_DIR}
  ${UPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
  ${upb_SOURCE_DIR}
)
target_link_libraries(comparison_benchmark PRIVATE 
  protobuf-nanopb-static
  upb
  port
  benchmark::benchmark
)

add_test(NAME nanopb_test COMMAND nanopb_test)
add_test(NAME upb_test COMMAND upb_test)
add_test(NAME comparison_test COMMAND comparison_test)

# Add custom target to show binary sizes
add_custom_target(show_sizes
  COMMAND ${CMAKE_COMMAND} -E echo "Binary size comparison:"
  COMMAND ${CMAKE_COMMAND} -E echo "UPB demo:"
  COMMAND ls -lh $<TARGET_FILE:upb_demo>
  COMMAND ${CMAKE_COMMAND} -E echo "Nanopb demo:"
  COMMAND ls -lh $<TARGET_FILE:nanopb_demo>
  DEPENDS upb_demo nanopb_demo
)
