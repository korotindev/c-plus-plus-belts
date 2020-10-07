file(GLOB PROTO_FILES "protos/*.proto")
file(GLOB TARGET_SRCS "*.h" "*.cpp")

if(PROTO_FILES)
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
endif()

add_library(belts-dev ${TARGET_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})

add_executable(${TARGET_RUNNER} ${TARGET_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})

add_library(${TARGET_LIB} INTERFACE)
target_include_directories(${TARGET_LIB}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

foreach(TARGET IN ITEMS ${TARGET_RUNNER} ${TARGET_LIB})
    target_include_directories(${TARGET} PRIVATE ${INCLUDE_DIRECTORY_PATH} ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${TARGET} ${Protobuf_LIBRARIES})
endforeach()