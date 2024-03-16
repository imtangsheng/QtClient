cmake_minimum_required(VERSION 3.14)

project(DataView LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui LinguistTools)

set(TS_FILES DataView_zh_CN.ts)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY /plugins)
add_library(DataView SHARED
  dataview.cpp
  dataview.h
  DataView.json
  ${TS_FILES}
)

set_target_properties(DataView PROPERTIES
    PREFIX "" #默认位lib，不设置空的话
)

target_link_libraries(DataView PRIVATE Qt${QT_VERSION_MAJOR}::Gui)

target_compile_definitions(DataView PRIVATE DATAVIEW_LIBRARY)

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 添加include目录
target_include_directories(DataView PRIVATE
    ../QtClient
)
