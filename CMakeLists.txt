cmake_minimum_required(VERSION 2.8)

# set(CMAKE_CXX_STANDARD 17)

project(object_detection)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

add_executable(object_detection object_detection.cpp)

target_link_libraries(object_detection ${OpenCV_LIBS})