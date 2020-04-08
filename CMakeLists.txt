cmake_minimum_required(VERSION 3.16)
project(c-plus-plus-belts)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(utils)

add_executable(c-plus-plus-belts main.cpp)
target_compile_options(c-plus-plus-belts PUBLIC -O2)
target_compile_options(c-plus-plus-belts PUBLIC -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)

target_link_libraries(c-plus-plus-belts INTERFACE utils)

target_include_directories(c-plus-plus-belts PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/utils")