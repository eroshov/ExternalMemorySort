cmake_minimum_required(VERSION 3.19)
project(sort)

set(CMAKE_CXX_STANDARD 20)

add_library(sort
        sort_performer.cpp
        file_utils.cpp)

add_compile_options(
        -Werror

        -Wall
        -Wextra
        -Wpedantic

        -O3
)
