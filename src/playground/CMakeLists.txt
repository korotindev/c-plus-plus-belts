file(GLOB_RECURSE PLAYGROUND_SRCS "*.h" "*.cpp")
add_executable(playground ${PLAYGROUND_SRCS})
target_compile_features(playground PUBLIC cxx_std_17)
target_compile_options(playground PUBLIC 
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