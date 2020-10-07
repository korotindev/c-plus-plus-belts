add_library(grader INTERFACE)
target_sources(grader INTERFACE profile.h test_runner.h)
target_include_directories(grader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})