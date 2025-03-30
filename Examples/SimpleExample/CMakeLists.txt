cmake_minimum_required(VERSION 3.19)


include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME Simple
    NAMESPACE Examples
    EXECUTABLE
    SOURCES main.cpp
    BUILD_DEPENDENCIES
        glm
        glfw
        RHI::RHI
        RHI::WebGPU
)

# target_link_libraries(Triangle fastgltf)

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Basic.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/ImGui.slang
)

set(SHADER_LIB_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/BxDF.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/DirectLighting.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Public/GpuScene.slang
)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    DEPENDS ${SHADER_FILES}
    TARGET Simple
    NAME Basic
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders
    SHADER_GFX_FILES ${SHADER_FILES}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/Shaders
    DEPENDENCIES ${SHADER_LIB_FILES}
)

# target_compile_definitions(Triangle PUBLIC GLM_FORCE_LEFT_HANDED)
# target_compile_definitions(Triangle PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)


# add_subdirectory(ImGui)
# target_link_libraries(Triangle ImGui)