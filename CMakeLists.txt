 cmake_minimum_required(VERSION 3.22)
project(daycounter)

set(CMAKE_CXX_STANDARD 17)

add_library(preloader SHARED IMPORTED)
set_target_properties(preloader PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/libpreloader.so
    IMPORTED_NO_SONAME TRUE)

add_library(daycounter SHARED src/main.cpp)

target_link_libraries(daycounter
    log
    dl
    preloader
)
