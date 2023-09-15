cmake_minimum_required(VERSION 3.16)

# Get VmaAllocator
set(VMA_RECORDING_ENABLED TRUE)
include(FetchContent)
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.0.1
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

# # Find Vulkan
find_package(Vulkan REQUIRED)

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI-Vulkan/Loader.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FrameScheduler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ResourcePool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ShaderBindGroup.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FrameScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ResourcePool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ShaderBindGroup.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME Vulkan
    NAMESPACE RHI
    SHARED
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        Vulkan::Vulkan
        VulkanMemoryAllocator
)
