cmake_minimum_required(VERSION 3.10)

project(model-scanner VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig)
find_package(OpenCV 4 REQUIRED)
pkg_search_module(apriltag REQUIRED apriltag)

include_directories(
  include
  ${OpenCV_INCLUDE_DIRS}
  ${apriltag_INCLUDE_DIRS}
)

file(GLOB model_scanner_SRC "src/*.cpp")

add_executable(model-scanner ${model_scanner_SRC})

target_link_libraries(model-scanner
  ${OpenCV_LIBRARIES}
  ${apriltag_LIBRARIES}
)
