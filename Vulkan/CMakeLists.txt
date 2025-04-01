cmake_minimum_required(VERSION 3.19)

# # Find Vulkan
find_package(Vulkan REQUIRED)

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/BindGroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DeleteQueue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraphPass.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/StagingBuffer.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/BindGroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DeleteQueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraphPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Surface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/StagingBuffer.cpp
)

if(NOT TARGET TL)
    include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
else()
    include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
endif()

tl_add_target(
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

target_compile_options(Vulkan PRIVATE ${RHI_COMPILE_FLAGS})
