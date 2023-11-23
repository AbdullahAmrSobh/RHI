cmake_minimum_required(VERSION 3.16)

if (MSVC)
#   set(CMAKE_CXX_FLAGS /WX)
endif()

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
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FrameGraph.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FrameGraph.cpp
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
