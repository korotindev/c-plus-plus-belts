enable_testing()

file(GLOB_RECURSE TEST_SRCS "*.h" "*.cpp")

add_executable(belts-unit-tests ${TEST_SRCS})

target_link_libraries(belts-unit-tests PRIVATE belts)

add_test(NAME test1 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/test/samples/test1)
add_test(NAME test2 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/test/samples/test2)
add_test(NAME test3 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/test/samples/test3)
add_test(NAME test4 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/test/samples/test4)