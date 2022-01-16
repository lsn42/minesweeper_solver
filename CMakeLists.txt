# CMake and project configuration
cmake_minimum_required(VERSION 3.11)
project(minesweeper_solver)

# source files
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(minesweeper_solver ${SOURCES})

# defined headers
file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/include)
target_include_directories(minesweeper_solver PRIVATE ${HEADERS})

# load libraries OpenCV
find_package(OpenCV 4 REQUIRED)
if(OpenCV_FOUND)
  message(STATUS "OpenCV library found, OpenCV_DIR = ${OpenCV_DIR}")
  target_include_directories(minesweeper_solver PRIVATE ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(minesweeper_solver ${OpenCV_LIBS})
else(OpenCV_FOUND)
  message(FATAL_ERROR "OpenCV library not found")
endif(OpenCV_FOUND)
