cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Basic.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME Basic
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    PUBLIC_INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/GLFW/include"
    BUILD_DEPENDENCIES
        RHI::RHI
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/GLFW/lib-vc2022/glfw3.lib"
    RUNTIME_LIBRARIES
        RHI::Vulkan
)