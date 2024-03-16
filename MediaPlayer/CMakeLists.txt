cmake_minimum_required(VERSION 3.14)

project(MediaPlayer VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui LinguistTools Widgets MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui LinguistTools Widgets MultimediaWidgets)

set(TS_FILES MediaPlayer_zh_CN.ts)
#LIBRARY_OUTPUT_DIRECTORY是一个属性，用于指定目标生成的库文件（DLL）的输出目录
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../plugins) #add_library 有效
#add_library(MediaPlayer SHARED
#  mediaplayer.cpp
#  mediaplayer.h
#  MediaPlayer.json
#  ${TS_FILES}
#)
#使用这个才可以修改目录
qt_add_plugin(MediaPlayer SHARED
    mediaplayer.cpp
    mediaplayer.h
    MediaPlayer.json
    ${TS_FILES}
)

target_sources(MediaPlayer
  PRIVATE
    MediaWidget.ui
)

set_target_properties(MediaPlayer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../plugins"
    PREFIX "" #默认位lib，不设置空的话
    OUTPUT_NAME "MediaPlayer"
)


target_link_libraries(MediaPlayer PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

target_compile_definitions(MediaPlayer PRIVATE MEDIAPLAYER_LIBRARY)

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 添加include目录
target_include_directories(MediaPlayer PRIVATE
    ../QtClient
)


