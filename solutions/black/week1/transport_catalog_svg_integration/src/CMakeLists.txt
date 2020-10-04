cmake_minimum_required(VERSION 3.16.3)
project(executor VERSION 0.1.0)

set(TARGET_RUNNER executor_runner)
set(TARGET_LIB executor)

file(GLOB TARGET_SRC "*.h" "*.cpp")

add_executable(${TARGET_RUNNER} ${TARGET_SRC})
add_library(${TARGET_LIB} ${TARGET_SRC})

foreach(TARGET IN ITEMS ${TARGET_RUNNER} ${TARGET_LIB})
    target_compile_features(${TARGET} PUBLIC cxx_std_20)
    target_compile_options(${TARGET} PUBLIC -Wall -Wextra -Werror -pedantic) 
    target_compile_definitions(${TARGET} PUBLIC -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_SANITIZE_VECTOR)
endforeach()

