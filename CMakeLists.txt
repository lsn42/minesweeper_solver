# CMake and project configuration
cmake_minimum_required(VERSION 3.5)
project(minesweeper_solver)

# source files
set(SOURCES ${PROJECT_SOURCE_DIR}/src/controller.cpp)
add_executable(minesweeper_solver ${SOURCES})

# defined headers
set(HEADERS ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/util)
target_include_directories(minesweeper_solver PRIVATE ${HEADERS})

# load libraries
find_package(OpenCV 4 REQUIRED)
if(OpenCV_FOUND)
  message(STATUS "OpenCV library found")
  message(STATUS "OpenCV_DIR = ${OpenCV_DIR}")
  # message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
  # message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
  target_include_directories(minesweeper_solver PRIVATE ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(minesweeper_solver ${OpenCV_LIBS})
else(OpenCV_FOUND)
  message(FATAL_ERROR "OpenCV library not found")
endif(OpenCV_FOUND)