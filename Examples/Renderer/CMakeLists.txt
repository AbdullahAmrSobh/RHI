cmake_minimum_required(VERSION 3.19)


set(RENDERER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(RENDERER_HEADER_FILES
        ${RENDERER_INCLUDE_DIR}/Common.h
        ${RENDERER_INCLUDE_DIR}/Geometry.h
        ${RENDERER_INCLUDE_DIR}/PipelineLibrary.h
        ${RENDERER_INCLUDE_DIR}/Renderer.h
        ${RENDERER_INCLUDE_DIR}/Resources.h
        ${RENDERER_INCLUDE_DIR}/ShaderParameters.h
        ${RENDERER_INCLUDE_DIR}/ImGuiPass.hpp
        ${RENDERER_INCLUDE_DIR}/Scene.hpp
)

set(RENDERER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)
set(RENDERER_SOURCE_FILES
    ${RENDERER_SOURCE_DIR}/DeferredRenderer.cpp
    ${RENDERER_SOURCE_DIR}/DeferredRenderer.hpp
    ${RENDERER_SOURCE_DIR}/Geometry.cpp
    ${RENDERER_SOURCE_DIR}/ImGuiPass.cpp
    ${RENDERER_SOURCE_DIR}/PipelineLibrary.cpp
    ${RENDERER_SOURCE_DIR}/Renderer.cpp
    ${RENDERER_SOURCE_DIR}/Resources.cpp
    ${RENDERER_SOURCE_DIR}/Scene.cpp
)

set(RENDERER_SHADERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include)
set(RENDERER_SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Source)
set(RENDERER_SHADER_FILES
    ${RENDERER_SHADERS_DIR}/GBufferPass.slang
    ${RENDERER_SHADERS_DIR}/ImGui.slang
    ${RENDERER_SHADERS_DIR}/Compose.slang
    ${RENDERER_SHADERS_DIR}/Cull.slang
)

# @note: for some reason I need to copy dawn dlls next to every executable (the build script should handle this!)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME
        Renderer
    NAMESPACE
        neon
    STATIC
    SOURCES
        ${RENDERER_INCLUDE_DIR}
        ${RENDERER_SOURCE_FILES}
)

# add include directories
target_include_directories(Renderer PUBLIC ${RENDERER_INCLUDE_DIR})
target_include_directories(Renderer PUBLIC ${RENDERER_SHADERS_INCLUDE_DIR})
target_include_directories(Renderer PRIVATE ${RENDERER_SOURCE_DIR})
target_include_directories(Renderer PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include/")
target_include_directories(Renderer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/")

include(${RHI_SOURCE_DIR}/cmake/compile-shaders-with-headers.cmake)
compile_shaders_with_headers(
    TARGET
        Renderer
    SHADER_FILES
        ${RENDERER_SHADER_FILES}
    INCLUDE_DIRS
        ${RENDERER_SHADERS_INCLUDE_DIR}
    OUTPUT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/Shaders"
    HEADER_OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/include/Renderer-Shaders
)

if (RHI_BACKEND_VULKAN)
    target_link_libraries(Renderer RHI::Vulkan)
endif()

if (RHI_BACKEND_WEBGPU)
    target_link_libraries(Renderer RHI::WebGPU)
endif()

if (RHI_BACKEND_D3D12)
    target_link_libraries(Renderer RHI::D3D12)
endif()

target_link_libraries(Renderer TL)

target_link_libraries(Renderer Examples::Base)

target_link_libraries(Renderer rhi_imgui)
