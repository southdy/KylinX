cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../code")

add_definitions(-DDEBUG=1)
add_definitions(-DANDROID)
add_definitions(-DGL_GLEXT_PROTOTYPES)

message(STATUS "Current build type: ${CMAKE_BUILD_TYPE}")

file(GLOB SOURCE_FILES_KYLIN
  ${PROJ_DIR}/kylin/*.c
)

add_library(
  kylin SHARED
  ${SOURCE_FILES_KYLIN}
)

target_include_directories(
  kylin PRIVATE
  .
  ${PROJ_DIR}/kylin
  ${PROJ_DIR}/thirdparty
)

target_link_libraries(
  kylin
  android
  EGL
  GLESv1_CM
  GLESv2
  OpenSLES
  log
)
