cmake_minimum_required(VERSION 3.19)

set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/BindGroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RenderTarget.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Handle.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Image.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RenderGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/PipelineAccess.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Result.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RGPass.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RGResources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/SampleCount.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Sampler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Swapchain.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/OffsetAllocator/offsetAllocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/OffsetAllocator/offsetAllocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/BindGroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RGPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RGResources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Sampler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME RHI
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)

target_compile_options(RHI PRIVATE ${RHI_COMPILE_FLAGS})

target_link_libraries(RHI PUBLIC TL)