cmake_minimum_required(VERSION 3.16.3)
project(freq VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-recover=address")

#set(BOOST_DEBUG ON)
#find_package(Boost 1.71.0)
#include_directories(${Boost_INCLUDE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_SANITIZE_VECTOR)

include (cmake/cppcheck.cmake)
include (cmake/clang-format.cmake)

add_custom_target(debug_run
  COMMAND LSAN_OPTIONS=max_leaks=1 ASAN_OPTIONS=debug=1:detect_leaks=1:abort_on_error=1:halt_on_error=1:verbosity=2:color=always bin/freq 
  DEPENDS bin/freq
)

add_custom_target(run 
  COMMAND bin/freq
  DEPENDS bin/freq
)

add_executable(freq src/main.cpp)
target_compile_features(freq PUBLIC cxx_std_17)
target_link_libraries(freq pthread)