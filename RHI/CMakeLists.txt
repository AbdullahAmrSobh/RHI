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

set(RHI_BACKEND_WEBGPU_SOURCES
    # Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/resources.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/command-list.h
    # Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/webgpu/command-list.cpp
)

set(RHI_BACKEND_D3D12_SOURCES
    # Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/resources.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/command-list.h
    # Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/device-stubs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/resources-stubs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/backends/D3D12/command-list-stubs.cpp
)
set(RHI_BACKEND_D3D12_LIBRARIES
    D3D12MemoryAllocator
    d3d12
    dxgi
    dxguid
)

# Only one backend may be compiled into RHI.cpp's dispatch table at a time.
# Keep the Vulkan default, then select D3D12 or WebGPU when Vulkan is disabled.
if(RHI_BACKEND_VULKAN)
    set(RHI_COMPILE_BACKEND_DEFINE RHI_COMPILE_BACKEND_VULKAN=1)
elseif(RHI_BACKEND_D3D12)
    set(RHI_COMPILE_BACKEND_DEFINE RHI_COMPILE_BACKEND_D3D12=1)
elseif(RHI_BACKEND_WEBGPU)
    set(RHI_COMPILE_BACKEND_DEFINE RHI_COMPILE_BACKEND_WEBGPU=1)
endif()

add_library(RHI STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Reflect.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI.cpp
    $<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,${RHI_BACKEND_VULKAN_SOURCES},>
    $<IF:$<BOOL:${RHI_BACKEND_D3D12}>,${RHI_BACKEND_D3D12_SOURCES},>
    $<IF:$<BOOL:${RHI_BACKEND_WEBGPU}>,${RHI_BACKEND_WEBGPU_SOURCES},>
)
add_library(RHI::RHI ALIAS RHI)

target_include_directories(RHI
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/Export>"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PRIVATE $<$<BOOL:${RHI_BACKEND_D3D12}>:${RHI_dma_SOURCE_DIR}/include>
)

include(GenerateExportHeader)
generate_export_header(RHI
    EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Export.hpp
    EXPORT_MACRO_NAME RHI_EXPORT
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
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
        $<$<BOOL:${RHI_TRACY_ENABLE}>:TRACY_ENABLE>
        $<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,${RHI_BACKEND_VULKAN_COMPILE_DEFINITIONS},>

        ${RHI_COMPILE_BACKEND_DEFINE}
)

target_link_libraries(RHI
    PUBLIC
        TL
    PRIVATE
        $<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,${RHI_BACKEND_VULKAN_LIBRARIES},>
        $<IF:$<BOOL:${RHI_BACKEND_D3D12}>,${RHI_BACKEND_D3D12_LIBRARIES},>
)

if (RHI_BACKEND_WEBGPU)
    if(EMSCRIPTEN)
        add_dependencies(RHI emdawnwebgpu_pkg)
        target_compile_options(RHI PRIVATE "--use-port=${RHI_EMDAWNWEBGPU_PORT}")
    else()
        target_link_libraries(RHI PRIVATE dawn::webgpu_dawn)
    endif()
endif()
