cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ApplicationBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Window.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME Base
    NAMESPACE Examples
    STATIC
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        glm
        glfw
        TL
)