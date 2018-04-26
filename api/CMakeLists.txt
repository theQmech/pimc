cmake_minimum_required (VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIN64 -std=c++11" )
include_directories("${ABC_DIR}/src")

add_library(api api.cpp utils.cpp)
