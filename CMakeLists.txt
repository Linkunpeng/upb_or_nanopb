cmake_minimum_required(VERSION 3.15)
project(upb_nanopb_comparison CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch protobuf (which includes upb)
FetchContent_Declare(
  protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG v25.1
)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)

# Make protobuf available
FetchContent_GetProperties(protobuf)
if(NOT protobuf_POPULATED)
  FetchContent_Populate(protobuf)
  # Add protobuf to the build
  add_subdirectory(${protobuf_SOURCE_DIR} ${protobuf_BINARY_DIR} EXCLUDE_FROM_ALL)
  
  # Manually add protoc-gen-upb as a target since protobuf's CMake doesn't export it
  # This is a workaround for the lack of CMake support for upb generators
  add_executable(protoc-gen-upb
    ${protobuf_SOURCE_DIR}/upb_generator/protoc-gen-upb.cc
    ${protobuf_SOURCE_DIR}/upb_generator/code_generator_request.c
    ${protobuf_SOURCE_DIR}/upb_generator/common.cc
    ${protobuf_SOURCE_DIR}/upb_generator/file_layout.cc
    ${protobuf_SOURCE_DIR}/upb_generator/keywords.cc
    ${protobuf_SOURCE_DIR}/upb_generator/names.cc
    ${protobuf_SOURCE_DIR}/upb_generator/subprocess.cc
  )
  target_link_libraries(protoc-gen-upb PRIVATE 
    libprotoc 
    libprotobuf
    upb_base_lib
    upb_mem_lib  
    upb_message_lib
    upb_mini_descriptor_lib
    upb_mini_table_lib
  )
  target_include_directories(protoc-gen-upb PRIVATE 
    ${protobuf_SOURCE_DIR}
    ${protobuf_SOURCE_DIR}/upb
  )
endif()

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

# Generate upb files using protobuf's upb plugin
set(UPB_OUT_DIR ${CMAKE_BINARY_DIR}/generated/upb)
file(MAKE_DIRECTORY ${UPB_OUT_DIR})

# Protobuf v25+ includes upb code generator
# We'll use protoc with --upb_out
add_custom_command(
  OUTPUT ${UPB_OUT_DIR}/person.upb.h ${UPB_OUT_DIR}/person.upb.c
  COMMAND ${PROTOC}
    --plugin=protoc-gen-upb=$<TARGET_FILE:protoc-gen-upb>
    --upb_out=${UPB_OUT_DIR}
    -I${CMAKE_SOURCE_DIR}/proto
    ${PROTO_FILE}
  DEPENDS ${PROTO_FILE} protoc-gen-upb
  COMMENT "Generating upb files with protoc-gen-upb"
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
  ${protobuf_SOURCE_DIR}
  ${protobuf_SOURCE_DIR}/upb
)
target_link_libraries(upb_demo PRIVATE 
  upb_base_lib
  upb_mem_lib
  upb_message_lib
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
  ${protobuf_SOURCE_DIR}
  ${protobuf_SOURCE_DIR}/upb
)
target_link_libraries(upb_test PRIVATE 
  upb_base_lib
  upb_mem_lib
  upb_message_lib
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
  ${protobuf_SOURCE_DIR}
  ${protobuf_SOURCE_DIR}/upb
)
target_link_libraries(comparison_test PRIVATE 
  protobuf-nanopb-static
  upb_base_lib
  upb_mem_lib
  upb_message_lib
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
  ${protobuf_SOURCE_DIR}
  ${protobuf_SOURCE_DIR}/upb
)
target_link_libraries(upb_benchmark PRIVATE 
  upb_base_lib
  upb_mem_lib
  upb_message_lib
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
  ${protobuf_SOURCE_DIR}
  ${protobuf_SOURCE_DIR}/upb
)
target_link_libraries(comparison_benchmark PRIVATE 
  protobuf-nanopb-static
  upb_base_lib
  upb_mem_lib
  upb_message_lib
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
