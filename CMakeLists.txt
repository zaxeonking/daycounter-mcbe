cmake_minimum_required(VERSION 3.22)
project(daycounter)

set(CMAKE_CXX_STANDARD 17)

file(GLOB IMGUI_SOURCES "imgui_src/*.cpp")

add_library(daycounter SHARED 
    src/main.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(daycounter PRIVATE imgui_src)

target_link_libraries(daycounter
    log
    dl
    EGL
    GLESv3
    ${CMAKE_SOURCE_DIR}/libs/libpreloader.so
)
