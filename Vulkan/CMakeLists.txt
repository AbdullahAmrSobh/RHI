cmake_minimum_required(VERSION 3.19)

find_package(Vulkan REQUIRED)

set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
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
        volk
        VulkanMemoryAllocator
)

target_compile_options(Vulkan PRIVATE ${RHI_COMPILE_FLAGS})

target_compile_definitions(Vulkan
    PRIVATE
        VK_NO_PROTOTYPES=1
        VK_USE_PLATFORM_WIN32_KHR=1
)