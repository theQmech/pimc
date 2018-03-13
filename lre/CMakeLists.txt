cmake_minimum_required (VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} "." ${CMAKE_BINARY_DIR}/lre)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIN64 -std=c++11" )

find_package(BISON)
find_package(FLEX)
bison_target(myparser parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cc)
flex_target(myscanner scanner.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.cc)
# needed because scanner uses token declarations from parser
add_flex_bison_dependency(myscanner myparser)

add_library(lre
	${FLEX_myscanner_OUTPUTS} ${BISON_myparser_OUTPUTS} ${BISON_myparser_HEADER}
	driver.cc symbols.cc symtab.cc driver.hh symbols.h
)

