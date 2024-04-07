# Copyright (C) 2024 The DCRobot Company Ltd.
# Author: Tang
# Created: 2024-03

cmake_minimum_required(VERSION 3.5)
project(QtClientStudio LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_subdirectory(QtClient)
#add_subdirectory(MediaPlayer)
#add_subdirectory(DataView)
add_subdirectory(ShunDe)
#add_library(QtClientStudio SHARED
#    readme.md
#)
