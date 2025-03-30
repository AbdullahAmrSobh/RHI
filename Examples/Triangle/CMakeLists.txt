cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/ImGuiRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/PipelineLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/BufferPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/OffsetAllocator/OffsetAllocator.cpp
)

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Basic.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/ImGui.slang
)

set(SHADER_LIB_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/BxDF.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/DirectLighting.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Public/GpuScene.slang
)

# @note: for some reason I need to copy dawn dlls next to every executable (the build script should handle this!)

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
        RHI::WebGPU
)

target_link_libraries(Triangle fastgltf)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    DEPENDS ${SHADER_FILES}
    TARGET Triangle
    NAME Basic
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders
    SHADER_GFX_FILES ${SHADER_FILES}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/Shaders
    DEPENDENCIES ${SHADER_LIB_FILES}
)

target_compile_definitions(Triangle PUBLIC GLM_FORCE_LEFT_HANDED)
target_compile_definitions(Triangle PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)


add_subdirectory(ImGui)
target_link_libraries(Triangle ImGui)