cmake_minimum_required(VERSION 3.15)
project(upb_nanopb_comparison CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch upb
FetchContent_Declare(
  upb
  GIT_REPOSITORY https://github.com/protocolbuffers/upb.git
  GIT_TAG main
)

# Fetch nanopb
FetchContent_Declare(
  nanopb
  GIT_REPOSITORY https://github.com/nanopb/nanopb.git
  GIT_TAG master
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

# Generate upb files
set(UPB_OUT_DIR ${CMAKE_BINARY_DIR}/generated/upb)
file(MAKE_DIRECTORY ${UPB_OUT_DIR})

add_custom_command(
  OUTPUT ${UPB_OUT_DIR}/person.upb.h ${UPB_OUT_DIR}/person.upb.c
  COMMAND ${PROTOC} --upb_out=${UPB_OUT_DIR} --plugin=protoc-gen-upb=$<TARGET_FILE:protoc-gen-upb> -I${CMAKE_SOURCE_DIR}/proto ${PROTO_FILE}
  DEPENDS ${PROTO_FILE} protoc-gen-upb
  COMMENT "Generating upb files"
)

# Generate nanopb files
set(NANOPB_OUT_DIR ${CMAKE_BINARY_DIR}/generated/nanopb)
file(MAKE_DIRECTORY ${NANOPB_OUT_DIR})

add_custom_command(
  OUTPUT ${NANOPB_OUT_DIR}/person.pb.h ${NANOPB_OUT_DIR}/person.pb.c
  COMMAND ${PROTOC} --nanopb_out=${NANOPB_OUT_DIR} --plugin=protoc-gen-nanopb=$<TARGET_FILE:protoc_nanopb_plugin> -I${CMAKE_SOURCE_DIR}/proto ${PROTO_FILE}
  DEPENDS ${PROTO_FILE} protoc_nanopb_plugin
  COMMENT "Generating nanopb files"
)

# UPB Example
add_executable(upb_demo
  src/upb_example/main.c
  ${UPB_OUT_DIR}/person.upb.c
)
target_include_directories(upb_demo PRIVATE ${UPB_OUT_DIR})
target_link_libraries(upb_demo PRIVATE upb)

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

# Benchmarks
add_executable(comparison_benchmark
  benchmarks/benchmark.cpp
  ${UPB_OUT_DIR}/person.upb.c
  ${NANOPB_OUT_DIR}/person.pb.c
)
target_include_directories(comparison_benchmark PRIVATE 
  ${UPB_OUT_DIR}
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
)
target_link_libraries(comparison_benchmark PRIVATE 
  upb
  protobuf-nanopb-static
  benchmark::benchmark
)

# Tests
add_executable(comparison_test
  tests/test.cpp
  ${UPB_OUT_DIR}/person.upb.c
  ${NANOPB_OUT_DIR}/person.pb.c
)
target_include_directories(comparison_test PRIVATE 
  ${UPB_OUT_DIR}
  ${NANOPB_OUT_DIR}
  ${nanopb_SOURCE_DIR}
)
target_link_libraries(comparison_test PRIVATE 
  upb
  protobuf-nanopb-static
  gtest
  gtest_main
)

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
