cmake_minimum_required(VERSION 3.14)

project(ShunDe LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui LinguistTools Widgets MultimediaWidgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui LinguistTools Widgets MultimediaWidgets Sql)

include_directories("./")
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE UI ./ui/*.*)
file(GLOB_RECURSE Modules ./modules/*.*)
file(GLOB_RECURSE Function ./function/*.*)

set(PUBLIC
    ../public/PluginInterface.h
    ../public/AppOS.h
    ../public/AppUtil.h ../public/AppUtil.cpp
    ../public/widgets/videowidget.cpp ../public/widgets/videowidget.h
    ../public/widgets/ellipsewidget.cpp ../public/widgets/ellipsewidget.h
    ../public/modules/sqlite.h ../public/modules/sqlite.cpp ../public/modules/sqlite.ui
)
set(RESOURCES
    readme.md
    resource.qrc
    doc/dev.md
)

set(TS_FILES ShunDe_zh_CN.ts)

source_group("UI" FILES ${UI})
source_group("Modules" FILES ${Modules})
source_group("Function" FILES ${Function})
source_group("Resource" FILES ${RESOURCES})
source_group("Public" FILES ${PUBLIC})

qt_add_plugin(ShunDe SHARED
  ShunDe.cpp
  ShunDe.h
  ShunDe.json
  ${TS_FILES}
)

target_sources(ShunDe PRIVATE
    ${UI} ${Modules} ${Function}
    ${PUBLIC} ${RESOURCES}

)

set_target_properties(ShunDe PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../plugins"
    PREFIX "" #默认位lib，不设置空的话
    OUTPUT_NAME "ShunDe"
)

target_link_libraries(ShunDe PRIVATE 
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::MultimediaWidgets
  Qt${QT_VERSION_MAJOR}::Sql
)

target_compile_definitions(ShunDe PRIVATE SHUNDE_LIBRARY)

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 添加include目录
target_include_directories(ShunDe PRIVATE
  ../public ../public/widgets/
)
