cmake_minimum_required(VERSION 3.10)
project(DyslexiaFocusGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(DyslexiaFocusGUI
    main.cpp
    MainWindow.cpp
    MainWindow.h
    DyslexiaLogic.cpp
    DyslexiaLogic.h
)

target_link_libraries(DyslexiaFocusGUI PRIVATE Qt6::Widgets)