cmake_minimum_required(VERSION 3.21)

project(video_call_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Allow Qt discovery via environment without hardcoding machine-specific paths.
if(DEFINED ENV{QT_ROOT} AND NOT "$ENV{QT_ROOT}" STREQUAL "")
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
endif()

add_subdirectory(client)
