cmake_minimum_required(VERSION 3.16)

if (MSVC)
#   set(CMAKE_CXX_FLAGS /WX)
endif()

# Add header files
set(HEADER_FILES
    # Common and utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Callstack.hpp
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

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common/Callstack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp

    ${CMAKE_SOURCE_DIR}/tracy/public/TracyClient.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
aams_add_target(
    NAME RHI
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)

target_include_directories(RHI SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/tracy/public")

target_link_libraries(RHI PUBLIC spirv-reflect-static)
target_link_libraries(RHI PUBLIC TL)
# target_link_libraries(RHI PUBLIC spirv-cross-reflect)
# target_link_libraries(RHI PUBLIC spirv-cross-glsl)

