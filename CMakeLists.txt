cmake_minimum_required(VERSION 3.16)
project(RedBelt)

set(CMAKE_CXX_STANDARD 20)

add_library(Utils INTERFACE)
target_sources(Utils INTERFACE test_runner.h log_duration.h)
target_compile_options(Utils INTERFACE -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)

add_executable(RedBelt main.cpp)
target_compile_options(RedBelt PUBLIC -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)

target_link_libraries(RedBelt INTERFACE Utils)