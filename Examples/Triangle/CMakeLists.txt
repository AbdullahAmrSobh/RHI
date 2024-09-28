cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
)

set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Examples/RPI/Shaders/Basic.slang
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME Triangle
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        # RPI
        Examples::Base
        # Assets
    RUNTIME_LIBRARIES
        RHI::Vulkan
)
include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    TARGET Triangle
    INCLUDE_DIRS
        # TODO: fix this and use RPI path
        ${CMAKE_SOURCE_DIR}/Examples/RPI/Include
    SHADER_GFX_FILES
        ${SHADER_FILES}
    OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/Shaders
)