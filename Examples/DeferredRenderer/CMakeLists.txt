cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/DeferredRenderer.cpp
)

set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Examples/Shaders/ImGui.slang
    ${CMAKE_SOURCE_DIR}/Examples/Shaders/Basic.slang
)

include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
aams_add_target(
    NAME DeferredRenderer
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        Examples::Base
    RUNTIME_LIBRARIES
        RHI::Vulkan
)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    TARGET Base
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/Examples/Shaders/core
        ${CMAKE_SOURCE_DIR}/Examples/Base/Include
    SHADER_GFX_FILES
        ${SHADER_FILES}
    OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/Shaders
)

add_custom_command(TARGET DeferredRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/assimp/Bin/assimp-vc143-mtd.dll"
        $<TARGET_FILE_DIR:DeferredRenderer>
)