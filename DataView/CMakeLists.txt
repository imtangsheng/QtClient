# Copyright (C) 2024 The DCRobot Company Ltd.
# Author: Tang
# Created: 2024-04

cmake_minimum_required(VERSION 3.14)

project(DataView LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui LinguistTools Widgets Charts Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui LinguistTools Widgets Charts Network)

include_directories("./")
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB UI_HEADERS ./ui/*.h)
file(GLOB UI_SOURCES ./ui/*.cpp)
file(GLOB UI ./ui/*.ui)

#file(GLOB_RECURSE PUBLIC ../public/*)
set(PUBLIC
    ../public/PluginInterface.h
    ../public/AppOS.h

    ../public/modules/FilesUtil.h ../public/modules/FilesUtil.cpp ../public/modules/FilesUtil.ui
)

set(RESOURCES
    readme.md
    resource.qrc

)

set(TS_FILES DataView_zh_CN.ts)

source_group("UI" FILES ${UI} ${UI_HEADERS} ${UI_SOURCES})
source_group("Resource" FILES ${RESOURCES})
source_group("Public" FILES ${PUBLIC})

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY /plugins)
#add_library(DataView SHARED
qt_add_plugin(DataView SHARED
  dataview.cpp
  dataview.h
  DataView.json
  ${TS_FILES}
)
target_sources(DataView PRIVATE
    ${UI} ${UI_HEADERS} ${UI_SOURCES}
    ${PUBLIC} ${RESOURCES}
)
)

set_target_properties(DataView PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../plugins"
    PREFIX "" #默认位lib，不设置空的话
    OUTPUT_NAME "DataView"
)

target_link_libraries(DataView PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Network
)

target_compile_definitions(DataView PRIVATE DATAVIEW_LIBRARY)

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 添加include目录
target_include_directories(DataView PRIVATE
    ../QtClient ../public
)
