cmake_minimum_required(VERSION 3.21)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavutil libswscale)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(video_call_client_lib STATIC
    src/FrameHandler.cpp
    src/MainWindow.cpp
    src/VideoPreview.cpp
)
target_include_directories(video_call_client_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(video_call_client_lib
    PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        PkgConfig::FFMPEG
)

add_executable(video_call_client
    src/main.cpp
)

target_link_libraries(video_call_client PRIVATE video_call_client_lib)

if(APPLE)
    set_target_properties(video_call_client PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )

    # Ensure the macOS camera permission plugin is linked so QCameraPermission works at runtime.
    qt_import_plugins(video_call_client INCLUDE Qt6::QDarwinCameraPermissionPlugin)
endif()

qt_finalize_target(video_call_client)
