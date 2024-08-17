cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
)

set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Examples/Shaders/Compose.slang
    ${CMAKE_SOURCE_DIR}/Examples/Shaders/Basic.slang
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME Triangle
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RPI
        Examples::Base
    RUNTIME_LIBRARIES
        RHI::Vulkan
)
