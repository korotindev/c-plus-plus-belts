cmake_minimum_required(VERSION 3.16.3)
project(c-plus-plus-belts VERSION 0.1.0)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC _GLIBCXX_ASSERTIONS _GLIBCXX_SANITIZE_VECTOR)
endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")

find_package(Protobuf REQUIRED)

include (cmake/cppcheck.cmake)
include (cmake/clang-format.cmake)

enable_testing()

add_subdirectory(./src)
add_subdirectory(./tests)