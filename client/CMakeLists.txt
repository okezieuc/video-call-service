cmake_minimum_required(VERSION 3.21)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(video_call_client
    src/main.cpp
    src/FrameHandler.cpp
    src/FrameHandler.h
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(video_call_client PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

if(APPLE)
    set_target_properties(video_call_client PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )

    # Ensure the macOS camera permission plugin is linked so QCameraPermission works at runtime.
    qt_import_plugins(video_call_client INCLUDE Qt6::QDarwinCameraPermissionPlugin)
endif()

qt_finalize_target(video_call_client)
