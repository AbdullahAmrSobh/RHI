cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/OffsetAllocator/OffsetAllocator.cpp
)

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Basic.slang
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME Triangle
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        Examples::Base
    RUNTIME_LIBRARIES
        RHI::Vulkan
)

target_link_libraries(Triangle fastgltf)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    TARGET Triangle
    NAME Basic
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders
    SHADER_GFX_FILES ${SHADER_FILES}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/Shaders
)

target_compile_definitions(Triangle PUBLIC GLM_FORCE_LEFT_HANDED)
target_compile_definitions(Triangle PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
