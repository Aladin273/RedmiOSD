cmake_minimum_required(VERSION 3.26)
project(ryzenadj LANGUAGES CXX)

set(RYZENADJ_BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin CACHE STRING "" FORCE)

add_library(ryzenadj INTERFACE)
target_include_directories(ryzenadj INTERFACE include)
target_link_directories(ryzenadj INTERFACE lib)
target_link_libraries(ryzenadj INTERFACE libryzenadj)