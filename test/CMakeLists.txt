CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(gas-tests)

add_subdirectory(lib)

include_directories(
  ${kazmath_SOURCE_DIR}/src
  ${glhck_SOURCE_DIR}/include
  ${GLFW_SOURCE_DIR}/include
)

add_executable(gas-simple simple.c)
target_link_libraries(gas-simple gas glhck glfw ${GLFW_LIBRARIES})

add_executable(gas-compound compound.c)
target_link_libraries(gas-compound gas glhck glfw ${GLFW_LIBRARIES})

add_executable(gas-looping looping.c)
target_link_libraries(gas-looping gas glhck glfw ${GLFW_LIBRARIES})

add_executable(gas-pathfind pathfind.c)
target_link_libraries(gas-pathfind gas glhck glfw ${GLFW_LIBRARIES})
