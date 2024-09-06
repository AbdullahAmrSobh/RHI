cmake_minimum_required(VERSION 3.16)

set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/BindGroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Definitions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Fence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Image.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RenderGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Resources.hpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/OffsetAllocator/offsetAllocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/OffsetAllocator/offsetAllocator.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/Source/Buffer.cpp
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
