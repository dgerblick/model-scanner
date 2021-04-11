cmake_minimum_required(VERSION 3.10)

project(model-scanner VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(PkgConfig)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV 4 REQUIRED)
pkg_search_module(apriltag REQUIRED apriltag)

include_directories(
  include
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${glm_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${apriltag_INCLUDE_DIRS}
)

file(GLOB model_scanner_SRC "src/*.cpp")

add_executable(model-scanner ${model_scanner_SRC})

target_link_libraries(model-scanner
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${glm_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${apriltag_LIBRARIES}
)

add_custom_command(TARGET model-scanner POST_BUILD
  DEPENDS ${CMAKE_SOURCE_DIR}/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory              
          ${CMAKE_SOURCE_DIR}/shaders 
          ${CMAKE_CURRENT_BINARY_DIR}/shaders
  COMMENT "Copying shaders" VERBATIM
)
