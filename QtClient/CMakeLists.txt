# Copyright (C) 2024 The DCRobot Company Ltd.
# Author: Tang
# Created: 2024-03

cmake_minimum_required(VERSION 3.5)

project(QtClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools MultimediaWidgets Charts Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools MultimediaWidgets Charts Network)

include_directories("./")
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE UI ./ui/*.*)
file(GLOB_RECURSE PRIVATRE_PUBLIC ./public/*.*)

set(PUBLIC
    ../public/PluginInterface.h
    ../public/AppOS.h
)
set(RESOURCES
#    "${PROJECT_SOURCE_DIR}/../README.md"
    "readme.md"
    "resource.qrc"
    "doc/dev.md"
)

set(TS_FILES QtClient_zh_CN.ts)

source_group("UI" FILES ${UI})
source_group("Resource" FILES ${RESOURCES})
source_group("Public" FILES ${PUBLIC} ${PRIVATRE_PUBLIC})
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtClient
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        FirstShowWidget.h FirstShowWidget.cpp FirstShowWidget.ui
        ${PUBLIC} ${PRIVATRE_PUBLIC}
        ${UI}
        ${RESOURCES}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET QtClient APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
    qt_add_lupdate(QtClient TS_FILES ${TS_FILES})
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(QtClient SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(QtClient
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(QtClient PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QtClient)
endif()
set_target_properties(QtClient PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS QtClient
    BUNDLE DESTINATION . #BUNDLE DESTINATION选项用于指定应用程序捆绑包的安装目录
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} #LIBRARY DESTINATION选项用于指定库文件的安装目录
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} #RUNTIME DESTINATION选项用于指定可执行文件的安装目录
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtClient)
endif()

#by tang
#添加当前项目目录到include头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#设置软件版本
set(APP_VERSION_MAJOR 0)
set(APP_VERSION_MINOR 0)
set(APP_VERSION_PATCH 2)
# 定义 SOFTWARE_VERSION 宏
add_compile_definitions(APP_VERSION="${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")

#设置目标的属性，包括构建选项、目标类型和其他属性
set_target_properties(QtClient PROPERTIES
    WIN32_EXECUTABLE TRUE #指定示例目标是否为Windows可执行文件
    MACOSX_BUNDLE TRUE #指定示例目标是否为Mac应用程序捆绑包
)

# 添加include目录
target_include_directories(QtClient PRIVATE
    ../public
)

#检查是否使用了共享库构建
#如果是，则将共享插件作为构建示例目标的依赖项进行构建。这意味着在构建示例目标时，也会构建共享插件。
#如果不是使用共享库构建，即Qt是以静态方式构建的，那么它将链接库到示例目标中。这意味着在构建示例目标时，会将库静态链接到示例目标中。
if(QT6_IS_SHARED_LIBS_BUILD)
    # Build the shared plugin too when building this example target.
#    add_dependencies(QtClient DataView MediaPlayer)
     add_dependencies(QtClient ShunDe)
else()
    # Link the echoplugin if Qt is built statically.
    target_link_libraries(QtClient PRIVATE
        ShunDe
    )
endif()
#MediaPlayer
