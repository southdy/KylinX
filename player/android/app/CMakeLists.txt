cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../code")

add_definitions(-DDEBUG=1)
add_definitions(-DANDROID)
add_definitions(-DGL_GLEXT_PROTOTYPES)

message(STATUS "Current build type: ${CMAKE_BUILD_TYPE}")

file(GLOB SOURCE_FILES_KYLIN
  ${PROJ_DIR}/kylin/*.c
	${PROJ_DIR}/kylin/src/*.c
	${PROJ_DIR}/kylin/src/atomic/*.c
	${PROJ_DIR}/kylin/src/audio/*.c
	${PROJ_DIR}/kylin/src/audio/android/*.c
	${PROJ_DIR}/kylin/src/audio/openslES/*.c
	${PROJ_DIR}/kylin/src/core/android/*.c
	${PROJ_DIR}/kylin/src/cpuinfo/*.c
	${PROJ_DIR}/kylin/src/events/*.c
	${PROJ_DIR}/kylin/src/file/*.c
	${PROJ_DIR}/kylin/src/loadso/dlopen/*.c
	${PROJ_DIR}/kylin/src/power/*.c
	${PROJ_DIR}/kylin/src/power/android/*.c
	${PROJ_DIR}/kylin/src/filesystem/android/*.c
	${PROJ_DIR}/kylin/src/sensor/*.c
	${PROJ_DIR}/kylin/src/sensor/android/*.c
	${PROJ_DIR}/kylin/src/render/*.c
	${PROJ_DIR}/kylin/src/render/*/*.c
	${PROJ_DIR}/kylin/src/stdlib/*.c
	${PROJ_DIR}/kylin/src/thread/*.c
	${PROJ_DIR}/kylin/src/thread/pthread/*.c
	${PROJ_DIR}/kylin/src/timer/*.c
	${PROJ_DIR}/kylin/src/timer/unix/*.c
	${PROJ_DIR}/kylin/src/video/*.c
	${PROJ_DIR}/kylin/src/video/android/*.c
	${PROJ_DIR}/kylin/src/video/yuv2rgb/*.c
)

add_library(
  kylin SHARED
  ${SOURCE_FILES_KYLIN}
)

target_include_directories(
  kylin PRIVATE
  .
  ${PROJ_DIR}/kylin
  ${PROJ_DIR}/kylin/include
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
