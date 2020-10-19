file(GLOB_RECURSE TEST_SRCS "*.h" "*.cpp")

add_executable(belts-unit-tests ${TEST_SRCS})

target_link_libraries(belts-unit-tests PRIVATE belts)