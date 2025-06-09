cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Passes/CullPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Passes/GBufferPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Passes/ImGuiPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/PipelineLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/BufferPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Geometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/OffsetAllocator/OffsetAllocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Scene.cpp
)

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/GBufferPass.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/ImGui.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Compose.slang
)

set(SHADER_GFX_FILES
    # ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/BxDF.slang
    # ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/DirectLighting.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/ShaderBindingTable.slang
)

set(SHADER_COMPUTE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Cull.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Lighting.slang
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

target_link_libraries(Triangle assimp)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    get_filename_component(VULKAN_LIB_DIR "${Vulkan_LIBRARY}" DIRECTORY)
    target_link_libraries(Triangle "${VULKAN_LIB_DIR}/slangd.lib")
endif()

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

target_link_libraries(Triangle imgui)