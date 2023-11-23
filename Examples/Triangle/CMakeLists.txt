cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
)

set(SHADER_FILES_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/Triangle.hlsl
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME Triangle
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        Examples::Base
    RUNTIME_LIBRARIES
        RHI::Vulkan
)

include(${CMAKE_SOURCE_DIR}/CMake/RHICompileShaders.cmake)
RHICompileShaders(
    NAME Triangle-Shaders
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Resources/Shaders"
    TARGET Triangle
    SHADER_FILES ${SHADER_FILES_LIST}
)