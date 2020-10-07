add_library(grader INTERFACE)
target_sources(grader INTERFACE grader/profile.h grader/test_runner.h)
target_include_directories(grader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/grader)

file(GLOB_RECURSE BELTS_PROTOS "protos/*.proto")
protobuf_generate_cpp(BELTS_PROTO_SRCS BELTS_PROTO_HDRS ${BELTS_PROTOS})
add_library(belts-protobuf ${BELTS_PROTO_SRCS} ${BELTS_PROTO_HDRS})
target_include_directories(belts-protobuf PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(belts-protobuf PUBLIC ${Protobuf_LIBRARIES})

file(GLOB_RECURSE BELTS_HDRS "solution/*.h")
file(GLOB_RECURSE BELTS_SRCS "solution/*.cpp")

add_library(belts ${BELTS_HDRS} ${BELTS_SRCS})
target_include_directories(belts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/solution)
target_link_libraries(belts PUBLIC belts-protobuf grader)
target_compile_options(belts PRIVATE 
    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wenum-compare
    -Wfloat-equal
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
)