cmake_minimum_required(VERSION 3.16)

set(HEADER_FILES
    # Common and utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Hash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Result.hpp
    # main files
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Definitions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RenderGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Swapchain.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
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
