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

# Add custom target to show binary sizes
add_custom_target(show_sizes
  COMMAND ${CMAKE_COMMAND} -E echo "Binary size comparison:"
  COMMAND ${CMAKE_COMMAND} -E echo "Nanopb demo:"
  COMMAND ls -lh $<TARGET_FILE:nanopb_demo>
  DEPENDS nanopb_demo
)
