file(GLOB_RECURSE BELTS_SRCS "*.h" "*.cpp")
add_library(belts ${BELTS_SRCS})
target_include_directories(belts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
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