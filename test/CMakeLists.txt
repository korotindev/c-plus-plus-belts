
file(GLOB_RECURSE TEST_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/**/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp" "*.h" "*.cpp")

add_executable(belts-unit-tests ${TEST_SRCS})

target_link_libraries(belts-unit-tests PRIVATE belts)

add_test(NAME simple_test1 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/samples/simple_test1)
add_test(NAME simple_test2 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/samples/simple_test2)