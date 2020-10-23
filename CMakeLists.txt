cmake_minimum_required(VERSION 3.16.3)
project(c-plus-plus-belts VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

find_package(Protobuf REQUIRED)

add_subdirectory(./src)
add_subdirectory(./test)

include (cmake/clang-format.cmake)


if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  add_compile_options(-g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls)
  add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC _GLIBCXX_ASSERTIONS _GLIBCXX_SANITIZE_VECTOR)
endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")