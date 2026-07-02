set(RHI_BACKEND_VULKAN_SOURCES
    # Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/resources.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/command-list.h
    # Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/Vulkan/command-list.cpp
)
set(RHI_BACKEND_VULKAN_LIBRARIES
    volk
    VulkanMemoryAllocator
)
set(RHI_BACKEND_VULKAN_COMPILE_DEFINITIONS
    VK_NO_PROTOTYPES=1
    $<$<PLATFORM_ID:Windows>:VK_USE_PLATFORM_WIN32_KHR=1>
)

tl_add_target(
    NAME RHI
    NAMESPACE RHI
    STATIC
    HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Reflect.hpp
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI.cpp
        $<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,${RHI_BACKEND_VULKAN_SOURCES},>
)

target_compile_definitions(RHI
    PUBLIC
        RHI_DEBUG=$<IF:$<BOOL:${RHI_DEBUG}>,1,0>
        RHI_PLATFORM_WINDOWS=$<IF:$<BOOL:${RHI_PLATFORM_WINDOWS}>,1,0>
        RHI_PLATFORM_ANDROID=$<IF:$<BOOL:${RHI_PLATFORM_ANDROID}>,1,0>
        RHI_PLATFORM_LINUX=$<IF:$<BOOL:${RHI_PLATFORM_LINUX}>,1,0>
        RHI_BACKEND_D3D12=$<IF:$<BOOL:${RHI_BACKEND_D3D12}>,1,0>
        RHI_BACKEND_VULKAN=$<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,1,0>
        RHI_BACKEND_WEBGPU=$<IF:$<BOOL:${RHI_BACKEND_WEBGPU}>,1,0>
    PRIVATE
        TRACY_ENABLE=$<IF:$<BOOL:${TRACY_ENABLE}>,1,0>
        $<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,${RHI_BACKEND_VULKAN_COMPILE_DEFINITIONS},>

        # TODO: hardcoded for now
        RHI_COMPILE_BACKEND_VULKAN=1
)

target_link_libraries(RHI
    PUBLIC
        TL
    PRIVATE
        $<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,${RHI_BACKEND_VULKAN_LIBRARIES},>
)
