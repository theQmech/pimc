cmake_minimum_required (VERSION 2.6)

project(PIMC)
add_subdirectory(lre)
add_subdirectory(api)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIN64 -std=c++11" )

find_library(ABC_DIR NAMES abc)
find_library(LIBABC libabc.so ${ABC_DIR})
if(NOT LIBABC)
  message(FATAL_ERROR "ABC shared library not found, please check ABC_DIR path")
endif()

include_directories("${ABC_DIR}/src" "." "./lre" "./api"
	${CMAKE_BINARY_DIR}/lre)
add_executable(pimc main.cpp comm.cpp compute.cpp)
target_link_libraries(pimc lre api ${LIBABC})
