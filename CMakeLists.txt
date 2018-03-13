cmake_minimum_required (VERSION 2.6)

project(PIMC)
add_subdirectory(lre)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIN64 -std=c++11" )
set(ABC_DIR "/home/rganvir/btp/alanmi-abc-b158c6fc94e2")
find_library(LIBABC libabc.so ${ABC_DIR})
if(NOT LIBABC)
  message(FATAL_ERROR "ABC shared library not found")
endif()

include_directories("${ABC_DIR}/src" "." "./lre" ${CMAKE_BINARY_DIR}/lre)
include_directories (${CMAKE_BINARY_DIR})
add_library(clauses utils.cpp api.cpp)
add_executable(pimc main.cpp comm.cpp compute.cpp)
target_link_libraries(pimc lre clauses ${LIBABC})
