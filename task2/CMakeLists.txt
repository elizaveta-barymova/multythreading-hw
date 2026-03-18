cmake_minimum_required(VERSION 3.10)
project(ApplyFunction)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(apply_function INTERFACE)
target_link_libraries(apply_function INTERFACE Threads::Threads)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(apply_function_tests apply_function_test.cpp)

target_link_libraries(apply_function_tests
  PRIVATE
  apply_function
  GTest::gtest_main
  Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(apply_function_tests)

add_executable(apply_function_benchmark apply_function_benchmark.cpp)

target_link_libraries(apply_function_benchmark
  PRIVATE
  apply_function
  benchmark::benchmark
  Threads::Threads
)

target_compile_options(apply_function_benchmark PRIVATE -O3)