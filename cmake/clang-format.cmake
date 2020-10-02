find_program(CLANG_FORMAT NAMES clang-format)

# get all project files
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h)

add_custom_target(
        format
        COMMAND ${CLANG_FORMAT}
        --style=file
        --Werror
        --sort-includes
        -i
        ${ALL_SOURCE_FILES}
)
