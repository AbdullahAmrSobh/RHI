cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
)

set(SHADER_FILES_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/Triangle.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/ImGui.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/Compose.hlsl
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

set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Examples/Shaders/ImGui.slang
    ${CMAKE_SOURCE_DIR}/Examples/Shaders/Basic.slang
)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    TARGET Base
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/Examples/Shaders/core
    SHADER_GFX_FILES
        ${SHADER_FILES}
    OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/Shaders
)


add_custom_command(TARGET Triangle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/assimp/Bin/assimp-vc143-mtd.dll"
        $<TARGET_FILE_DIR:Triangle>
)