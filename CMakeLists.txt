cmake_minimum_required(VERSION 2.8)

project(object_detection)

find_package(OpenCV REQUIRED)

add_subdirectory(yolo)

add_executable(object_detection object_detection.cpp)

target_link_libraries(object_detection ${OpenCV_LIBS} yolo)