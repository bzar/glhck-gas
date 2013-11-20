CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(gas)

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

add_subdirectory(test)
