cmake_minimum_required(VERSION 3.16)
project(RedmiOSD LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

add_subdirectory(ThirdParty)

set(REDMI_OSD_HEADERS
    RedmiOSD.h 
)

set(REDMI_OSD_SOURCES
    Main.cpp
    RedmiOSD.cpp
)

#set(REDMI_OSD_RESOURCES 
#    Resources/Default.png
#    Resources/Quit.png
#    Resources/Silence.png
#    Resources/Turbo.png
#)

qt_add_executable(RedmiOSD ${REDMI_OSD_HEADERS} ${REDMI_OSD_SOURCES})
#qt_add_resources(RedmiOSD "RedmiOSD" PREFIX "/" FILES ${REDMI_OSD_RESOURCES})

target_link_libraries(RedmiOSD PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets QHotkey::QHotkey)
set_target_properties(RedmiOSD PROPERTIES WIN32_EXECUTABLE TRUE)

add_custom_command(TARGET RedmiOSD POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Tools ${CMAKE_CURRENT_BINARY_DIR}/Tools)
add_custom_command(TARGET RedmiOSD POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Resources ${CMAKE_CURRENT_BINARY_DIR}/Resources)
add_custom_command(TARGET RedmiOSD POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/Presets.json ${CMAKE_CURRENT_BINARY_DIR}/Presets.json)
add_custom_command(TARGET RedmiOSD POST_BUILD COMMAND windeployqt6 --no-translations $<TARGET_FILE:RedmiOSD>)