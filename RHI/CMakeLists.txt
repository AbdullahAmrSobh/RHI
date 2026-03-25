set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RenderGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/PipelineAccess.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Result.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Swapchain.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
)

tl_add_target(
    NAME RHI
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)

target_link_libraries(RHI PUBLIC TL)

target_compile_definitions(RHI
    INTERFACE
        RHI_PLATFORM_WINDOWS=$<IF:$<BOOL:${RHI_PLATFORM_WINDOWS}>,1,0>
        RHI_PLATFORM_ANDROID=$<IF:$<BOOL:${RHI_PLATFORM_ANDROID}>,1,0>
        RHI_PLATFORM_LINUX=$<IF:$<BOOL:${RHI_PLATFORM_LINUX}>,1,0>
        RHI_DEBUG=$<IF:$<BOOL:${RHI_DEBUG}>,1,0>
        RHI_BACKEND_D3D12=$<IF:$<BOOL:${RHI_BACKEND_D3D12}>,1,0>
        RHI_BACKEND_VULKAN=$<IF:$<BOOL:${RHI_BACKEND_VULKAN}>,1,0>
        RHI_BACKEND_WEBGPU=$<IF:$<BOOL:${RHI_BACKEND_WEBGPU}>,1,0>
    PRIVATE
        TRACY_ENABLE=$<IF:$<BOOL:${TRACY_ENABLE}>,1,0>
)
