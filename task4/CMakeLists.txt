cmake_minimum_required(VERSION 3.14)
project(condition_variable_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

option(USE_SYSTEM_GTEST "Use system installed Google Test" OFF)

if(USE_SYSTEM_GTEST)
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    
    add_executable(condition_variable_test test_condition_variable.cpp)
    target_link_libraries(condition_variable_test ${GTEST_LIBRARIES} pthread)
else()
    include(FetchContent)
    
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    
    FetchContent_MakeAvailable(googletest)
    
    add_executable(condition_variable_test test_condition_variable.cpp)
    target_link_libraries(condition_variable_test gtest_main gtest pthread)
endif()

enable_testing()
add_test(NAME condition_variable_test COMMAND condition_variable_test)

set_tests_properties(condition_variable_test PROPERTIES
    PASS_REGULAR_EXPRESSION "All tests passed"
    FAIL_REGULAR_EXPRESSION "FAILED"
)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS condition_variable_test
    COMMENT "Running tests with verbose output"
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")