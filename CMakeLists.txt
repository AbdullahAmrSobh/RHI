cmake_minimum_required(VERSION 3.14)

include(cmake/variables.cmake)

# Project declaration
project(
    RHI
    VERSION 0.1.0
    DESCRIPTION "Renderer Hardware Interface"
    HOMEPAGE_URL "https://github.com/AbdullahAmrSobh/RHI"
    LANGUAGES CXX
)

# Set C++ standard and extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable developer mode
set(RHI_DEVELOPER_MODE ${PROJECT_IS_TOP_LEVEL})

option(TRACY_ENABLE "Enable Tracy profiler" ${PROJECT_IS_TOP_LEVEL})
option(RHI_DEBUG "Enable RHI debug mode" ${PROJECT_IS_TOP_LEVEL})
option(RHI_BUILD_EXAMPLES "Build examples tree." ${PROJECT_IS_TOP_LEVEL})

if(TRACY_ENABLE)
    add_compile_definitions(TRACY_ENABLE)
endif()

if(RHI_DEBUG)
    add_compile_definitions(RHI_DEBUG)
endif()

# ---- In-source build guard ----
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(
        FATAL_ERROR
        "In-source builds are not supported. "
        "Please read the BUILDING document before trying to build this project. "
        "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
    )
endif()

# ---- Add dependencies ----
include(Packages.cmake)

# ---- Declare library ----
add_subdirectory(RHI)
add_subdirectory(Vulkan)

# ---- Examples ----
if(RHI_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

# ---- Developer mode ----
if(NOT RHI_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
        AUTHOR_WARNING
        "Developer mode is intended for developers of RHI"
    )
endif()

include(cmake/dev-mode.cmake)
