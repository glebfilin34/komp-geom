cmake_minimum_required(VERSION 3.16)
project(1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(SegmentPointPosition main.cpp)

target_link_libraries(SegmentPointPosition
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

