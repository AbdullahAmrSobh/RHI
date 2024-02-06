cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ApplicationBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ImGuiRenderer.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME Base
    NAMESPACE Examples
    STATIC
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        glfw
        glm
        assimp
        RHI::RHI
        ImGui
    RUNTIME_LIBRARIES
        RHI::Vulkan
)