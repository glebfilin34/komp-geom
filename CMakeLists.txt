cmake_minimum_required(VERSION 3.16)
project(2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(SegmentsIntersection main.cpp)

target_link_libraries(SegmentsIntersection Qt6::Core Qt6::Widgets)
