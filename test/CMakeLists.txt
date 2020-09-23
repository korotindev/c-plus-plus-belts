cmake_minimum_required(VERSION 3.16.3)
project(testor VERSION 0.1.0)

set(TARGET testor_runner)

file(GLOB TARGET_SRC "*.h" "*.cpp")

add_executable(${TARGET} ${TARGET_SRC})

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")

endforeach()

target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_compile_options(${TARGET} PUBLIC -Wall -Wextra -Werror -pedantic) 
target_compile_definitions(${TARGET} PUBLIC -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_SANITIZE_VECTOR)
target_include_directories(${TARGET} PUBLIC ./utils ${executor_SOURCE_DIR})
target_link_libraries(${TARGET} executor)

add_test(NAME all_tests COMMAND testor_runner)