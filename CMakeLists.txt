cmake_minimum_required(VERSION 3.10)

project(model-scanner VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(PkgConfig)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenCV 4 REQUIRED)
pkg_search_module(apriltag REQUIRED apriltag)

message(WARN ${OpenCV_INCLUDE_DIRS})

include_directories(
  include
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${apriltag_INCLUDE_DIRS}
)

file(GLOB model_scanner_SRC "src/*.cpp")

add_executable(model-scanner ${model_scanner_SRC})

target_link_libraries(model-scanner
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${apriltag_LIBRARIES}
)
