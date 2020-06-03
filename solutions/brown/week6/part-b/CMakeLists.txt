cmake_minimum_required(VERSION 3.16.5)
project(c-plus-plus-belts)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-unused-parameter -Wno-conversion -Wno-sign-conversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -g -fno-sanitize-recover=null")

add_executable(c-plus-plus-belts main.cpp Coordinate.h Coordinate.cpp Database.h Database.cpp CustomUtils.h CustomUtils.cpp Request.h Request.cpp ReadResponse.cpp ReadResponse.h Solution.cpp Solution.h)

target_link_libraries(c-plus-plus-belts INTERFACE utils)

target_include_directories(c-plus-plus-belts PUBLIC "${PROJECT_BINARY_DIR}" "utils")