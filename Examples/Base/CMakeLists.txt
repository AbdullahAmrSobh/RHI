cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ApplicationBase.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/Source/ImGuiRenderer.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/Source/SceneGraph.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
aams_add_target(
    NAME Base
    NAMESPACE Examples
    STATIC
    SOURCES ${SOURCE_FILES}
    PUBLIC_INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/assimp/Includes"
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/GLFW/include"
    BUILD_DEPENDENCIES
        glm
        RHI::RHI
        ImGui
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/assimp/Bin/assimp-vc143-mtd.lib"
        "${CMAKE_SOURCE_DIR}/Examples/Dependencies/GLFW/lib-vc2022/glfw3.lib"
    RUNTIME_LIBRARIES
        RHI::Vulkan
)