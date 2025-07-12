cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/ShaderAssets.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME ShaderCompiler
    NAMESPACE Engine
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        RHI::Vulkan
    )

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    get_filename_component(VULKAN_LIB_DIR "${Vulkan_LIBRARY}" DIRECTORY)
    target_link_libraries(ShaderCompiler "${VULKAN_LIB_DIR}/slangd.lib")
endif()