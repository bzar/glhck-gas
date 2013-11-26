CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(gas)

OPTION(GAS_BUILD_TESTS "Build GAS tests/examples" OFF)

set(GLHCK_BUILD_EXAMPLES OFF CACHE BOOL "Skip GLHCK examples")

add_subdirectory(lib)

include_directories(
  include
  ${kazmath_SOURCE_DIR}/src
  ${glhck_SOURCE_DIR}/include
)

file(GLOB SOURCES src/*.c)
add_definitions(-DGLHCK_KAZMATH_FLOAT -DUSE_SINGLE_PRECISION)

add_library(gas
    ${SOURCES}
)

if(GAS_BUILD_TESTS)
    add_subdirectory(test)
endif(GAS_BUILD_TESTS)
