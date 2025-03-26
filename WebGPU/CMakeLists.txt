cmake_minimum_required(VERSION 3.19)

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Swapchain.cpp
)


include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME WebGPU
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        webgpu_dawn
        dawn_common
        dawn_native
        dawn_glfw
        dawn_headers
)

target_compile_options(WebGPU PRIVATE ${RHI_COMPILE_FLAGS})
# target_link_libraries(WebGPU PRIVATE ${dawn_headers})
