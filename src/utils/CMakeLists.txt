cmake_minimum_required(VERSION 3.16.5)
project(utils)

set(CMAKE_CXX_STANDARD 17)

add_library(utils INTERFACE)
target_sources(utils INTERFACE test_runner.h profile.h)