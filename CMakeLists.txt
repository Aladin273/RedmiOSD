cmake_minimum_required(VERSION 3.16)
project(RedmiOSD LANGUAGES CXX)

include(CMake/qt.cmake)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

set(REDMI_OSD_HEADERS
    RedmiOSD.h 
)

set(REDMI_OSD_SOURCES
    Main.cpp
    RedmiOSD.cpp
)

set(REDMI_OSD_RESOURCES 
    Resources/silence.png
    Resources/turbo.png
    Resources/main.png
)

qt_add_executable(RedmiOSD ${REDMI_OSD_HEADERS} ${REDMI_OSD_SOURCES})
qt_add_resources(RedmiOSD "RedmiOSD" PREFIX "/" FILES ${REDMI_OSD_RESOURCES})

target_link_libraries(RedmiOSD PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(RedmiOSD PROPERTIES WIN32_EXECUTABLE TRUE)

add_custom_command(TARGET RedmiOSD POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Tools ${CMAKE_CURRENT_BINARY_DIR}/Tools)
add_custom_command(TARGET RedmiOSD POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/Presets.json ${CMAKE_CURRENT_BINARY_DIR}/Presets.json)
