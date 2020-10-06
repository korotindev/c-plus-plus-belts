cmake_minimum_required(VERSION 3.16.3)
project(executor VERSION 0.1.0)

file(GLOB PROTO_FILES "*.proto")
if(PROTO_FILES)
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
endif()

set(TARGET_RUNNER executor_runner)
set(TARGET_LIB executor)

file(GLOB TARGET_SRCS "*.h" "*.cpp")
add_executable(${TARGET_RUNNER} ${TARGET_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
add_library(${TARGET_LIB} ${TARGET_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})

foreach(TARGET IN ITEMS ${TARGET_RUNNER} ${TARGET_LIB})
    target_compile_features(${TARGET} PUBLIC cxx_std_20)
    target_compile_options(${TARGET} PUBLIC -Wall -Wextra -Werror -pedantic) 
    target_include_directories(${TARGET} PUBLIC ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${TARGET} ${Protobuf_LIBRARIES})
endforeach()