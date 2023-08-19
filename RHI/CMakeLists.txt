cmake_minimum_required(VERSION 3.16)

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Fence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Flag.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/FrameScheduler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Object.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Pass.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/PassProducer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Platform/
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Result.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/ShaderBindGroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Swapchain.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FrameScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ShaderBindGroup.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME RHI
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)
