cmake_minimum_required(VERSION 3.14)
project(parallel_closest_points_tester)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# # Include all .cpp files in this directory
# FILE(GLOB ALL_TEST_FILES *.cpp)
# # Include all the files in this directory that end with _test.cpp
# FILE(GLOB ALL_TEST_FILES *_test.cpp)
# Include all the files in this directory and its children that end with _test.cpp
FILE(GLOB_RECURSE ALL_TEST_FILES *_test.cpp)

add_executable(
  test_suite
  ${ALL_TEST_FILES}
)

target_link_libraries(
  test_suite
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_suite)
