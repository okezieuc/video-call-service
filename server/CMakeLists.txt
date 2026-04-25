cmake_minimum_required(VERSION 3.21)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(video_call_server
    src/ConnectionHandler.cpp
    src/TcpServer.cpp
    src/main.cpp
)

target_include_directories(video_call_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/shared
)

target_link_libraries(video_call_server PRIVATE
    Qt6::Core
    Qt6::Network
)

qt_finalize_target(video_call_server)
