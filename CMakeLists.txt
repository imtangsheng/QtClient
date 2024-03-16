# Copyright (C) 2024 The Qt Company Ltd.


cmake_minimum_required(VERSION 3.5)
project(QtClientStudio LANGUAGES CXX)




find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_subdirectory(QtClient)
add_subdirectory(MediaPlayer)
#add_subdirectory(DataView)
