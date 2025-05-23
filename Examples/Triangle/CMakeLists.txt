cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/ImGuiRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/PipelineLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/BufferPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/OffsetAllocator/OffsetAllocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Scene.cpp
)

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Basic.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/ImGui.slang
)

set(SHADER_GFX_FILES
    # ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/BxDF.slang
    # ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/DirectLighting.slang
    # ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Public/GpuScene.slang
)

set(SHADER_COMPUTE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Cull.slang
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
)

if (RHI_BACKEND_VULKAN)
    target_link_libraries(Triangle RHI::Vulkan)
endif()

if (RHI_BACKEND_WEBGPU)
    target_link_libraries(Triangle RHI::WebGPU)
endif()

if (RHI_BACKEND_D3D12)
    target_link_libraries(Triangle RHI::D3D12)
endif()


target_link_libraries(Triangle fastgltf)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    DEPENDS ${SHADER_FILES}
    TARGET Triangle
    NAME Basic
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/Shaders
    SHADER_GFX_FILES ${SHADER_FILES}
    SHADER_COMPUTE_FILES ${SHADER_COMPUTE_FILES}
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders
    DEPENDENCIES
        ${SHADER_GFX_FILES}
        ${SHADER_COMPUTE_FILES}
)

target_compile_definitions(Triangle PUBLIC GLM_FORCE_LEFT_HANDED)
target_compile_definitions(Triangle PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)


add_subdirectory(ImGui)
target_link_libraries(Triangle ImGui)