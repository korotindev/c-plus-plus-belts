file(GLOB_RECURSE PLAYGROUND_SRCS "*.h" "*.cpp")
add_executable(playground ${PLAYGROUND_SRCS})
target_compile_features(playground PUBLIC cxx_std_17)