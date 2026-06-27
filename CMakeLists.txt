cmake_minimum_required(VERSION 3.22)
project(daycounter)

set(CMAKE_CXX_STANDARD 17)

add_library(daycounter SHARED src/main.cpp)

target_link_libraries(daycounter
    log
    dl
    ${CMAKE_SOURCE_DIR}/libs/libpreloader.so
)
