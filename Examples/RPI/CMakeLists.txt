set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RPI/Renderer.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DeferredRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME RPI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        TL
        Examples::Base # needed for window
    RUNTIME_LIBRARIES
        RHI::Vulkan
)

include(${CMAKE_SOURCE_DIR}/cmake/compile-slang-shaders.cmake)
compile_slang_shaders(
    TARGET Base
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/Examples/Shaders/core
        ${CMAKE_SOURCE_DIR}/Examples/Base/Include
    SHADER_GFX_FILES
        ${SHADER_FILES}
    OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/Shaders
)
