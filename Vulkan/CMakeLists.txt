cmake_minimum_required(VERSION 3.16)

# Add dependencies
CPMAddPackage(
    NAME           RHI_vma
	GIT_REPOSITORY git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
	GIT_TAG        v3.0.1
)

# # Find Vulkan
find_package(Vulkan REQUIRED)

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraphCompiler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraphCompiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Surface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME Vulkan
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        Vulkan::Vulkan
        VulkanMemoryAllocator
)
