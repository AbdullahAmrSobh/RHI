cmake_minimum_required(VERSION 3.19)

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME D3D12
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)

target_link_libraries(
    D3D12
        RHI::RHI
        D3D12MemoryAllocator
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
)

target_include_directories(D3D12
    PUBLIC
        $<BUILD_INTERFACE:${D3D12MemoryAllocator_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:Include>
)

target_compile_options(D3D12 PRIVATE ${RHI_COMPILE_FLAGS})
