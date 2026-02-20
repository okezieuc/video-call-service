cmake_minimum_required(VERSION 3.21)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(video_call_client
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(video_call_client PRIVATE Qt6::Widgets)
