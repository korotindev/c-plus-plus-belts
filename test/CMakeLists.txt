
add_executable(belts-unit-tests main.cpp)

target_link_libraries(belts-unit-tests PRIVATE belts)

add_test(NAME test1 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/samples/test1)
add_test(NAME test2 COMMAND belts-unit-tests ${CMAKE_CURRENT_SOURCE_DIR}/samples/test2)