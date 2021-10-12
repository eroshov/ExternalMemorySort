cmake_minimum_required(VERSION 3.19)
project(sort)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(sort_tests test.cpp)

target_link_libraries(sort_tests
        gtest_main
        sort)

add_custom_target(test ./sort_tests)