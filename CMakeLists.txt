cmake_minimum_required(VERSION 3.16.5)
project(c-plus-plus-belts)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion -Wunused-variable -Wunused-parameter")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-unused-parameter -Wno-conversion -Wno-sign-conversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g -fno-sanitize-recover=null")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -g -fno-sanitize-recover=null")

add_library(coordinate Coordinate.h Coordinate.cpp)
list(APPEND EXTRA_LIBS coordinate)

add_library(database Database.h Database.cpp StopsStorage.cpp StopsStorage.h BusStorage.cpp BusStorage.h)
list(APPEND EXTRA_LIBS database)

add_library(custom_utils CustomUtils.h CustomUtils.cpp)
list(APPEND EXTRA_LIBS custom_utils)

add_library(request Request.h Request.cpp)
list(APPEND EXTRA_LIBS request)

add_library(read_response ReadResponse.cpp ReadResponse.h)
list(APPEND EXTRA_LIBS read_response)

add_library(json Json.cpp Json.h)
list(APPEND EXTRA_LIBS json)

add_library(settings Settings.h Settings.cpp)
list(APPEND EXTRA_LIBS settings)

add_library(graph INTERFACE)
target_sources(graph INTERFACE Graph.h Router.h)
list(APPEND EXTRA_LIBS graph)

add_library(solution Solution.cpp Solution.h)
target_link_libraries(solution INTERFACE utils)
target_include_directories(solution PUBLIC "${PROJECT_BINARY_DIR}" "utils")
list(APPEND EXTRA_LIBS solution)

add_executable(c-plus-plus-belts main.cpp)
target_link_libraries(c-plus-plus-belts INTERFACE utils)
target_link_libraries(c-plus-plus-belts PUBLIC ${EXTRA_LIBS})

target_include_directories(c-plus-plus-belts PUBLIC "${PROJECT_BINARY_DIR}" "utils")