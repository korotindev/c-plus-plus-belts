cmake_minimum_required(VERSION 3.16.3)
project(tests VERSION 0.1.0)

set(TARGET tests_runner)

file(GLOB TARGET_SRC "*.h" "*.hpp" "*.cpp")

add_executable(${TARGET} ${TARGET_SRC})

target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_compile_options(${TARGET} PUBLIC -Wall -Wextra -Werror -pedantic) 
target_include_directories(${TARGET} PUBLIC ./utils ${executor_SOURCE_DIR} ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${TARGET} executor ${Protobuf_LIBRARIES})


add_test(NAME test1 COMMAND ${TARGET})