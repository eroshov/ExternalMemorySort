cmake_minimum_required(VERSION 3.19)
project(sort)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

add_subdirectory(src)
add_subdirectory(tests)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(main main.cpp)

add_compile_options(-O3)

target_link_libraries(main sort)
