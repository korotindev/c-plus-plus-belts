cmake_minimum_required(VERSION 3.16.5)
project(c-plus-plus-belts)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-unused-parameter -Wno-conversion -Wno-sign-conversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -fno-sanitize-recover=null")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fsanitize-address-use-after-scope")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -g -fno-sanitize-recover=null")

add_executable(c-plus-plus-belts main.cpp Textures.cpp Textures.h Common.h Solution.cpp)

target_link_libraries(c-plus-plus-belts INTERFACE utils)

target_include_directories(c-plus-plus-belts PUBLIC "${PROJECT_BINARY_DIR}" "utils")