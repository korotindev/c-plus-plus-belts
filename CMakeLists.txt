cmake_minimum_required(VERSION 3.16)
project(belts)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(utils)

add_executable(belts main.cpp)
target_compile_options(belts PUBLIC -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)

target_link_libraries(belts INTERFACE utils)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(belts PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/utils")