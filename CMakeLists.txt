cmake_minimum_required(VERSION 3.19)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable developer mode
set(RHI_DEVELOPER_MODE ${PROJECT_IS_TOP_LEVEL})

option(TRACY_ENABLE       "Enable Tracy profiler"          ${PROJECT_IS_TOP_LEVEL})
option(RHI_DEBUG          "Enable RHI debug mode"          ${PROJECT_IS_TOP_LEVEL})

if(NOT EMSCRIPTEN)
    option(RHI_BACKEND_D3D12  "Enable RHI DirectX 12 Backend"  OFF) # TODO: Should enable later
    option(RHI_BACKEND_VULKAN "Enable RHI Vulkan 1.3 Backend"  ON)
    option(RHI_BACKEND_WEBGPU "Enable RHI WebGPU Backend"      OFF)
else()
    set(RHI_BACKEND_D3D12 OFF)
    set(RHI_BACKEND_VULKAN OFF)
    set(RHI_BACKEND_WEBGPU ON)
endif()

option(RHI_BUILD_EXAMPLES "Build examples tree."           ${PROJECT_IS_TOP_LEVEL})

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

# ---- Add Library Flags ----
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(RHI_COMPILE_FLAGS /sdl /guard:cf /utf-8 /diagnostics:caret /w14165 /w44242 /w44254 /w44263 /w34265 /w34287 /w44296 /w44365 /w44388 /w44464 /w14545 /w14546 /w14547 /w14549 /w14555 /w34619 /w34640 /w24826 /w14905 /w14906 /w14928 /w45038 /W4 /permissive- /volatile:iso /Zc:inline /Zc:preprocessor /Zc:enumTypes /Zc:lambda /Zc:__cplusplus /Zc:externConstexpr /Zc:throwingNew /EHsc)
    else(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(RHI_COMPILE_FLAGS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS=1 -fstack-protector-strong -fcf-protection=full -fstack-clash-protection -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wcast-qual -Wformat=2 -Wundef -Werror=float-equal -Wshadow -Wcast-align -Wunused -Wnull-dereference -Wdouble-promotion -Wimplicit-fallthrough -Wextra-semi -Woverloaded-virtual -Wnon-virtual-dtor -Wold-style-cast)
    add_compile_options(/fsanitize=address)
endif()

# ---- Declare library ----
add_subdirectory(RHI)

if(RHI_BACKEND_D3D12)
    # add_compile_definitions(RHI_BACKEND_D3D12)
    # add_subdirectory(D3D12)
endif()

if(RHI_BACKEND_VULKAN)
    add_compile_definitions(RHI_BACKEND_VULKAN)
    add_subdirectory(Vulkan)
endif()

if(RHI_BACKEND_WEBGPU)
    # add_compile_definitions(RHI_BACKEND_WEBGPU)
    # add_subdirectory(WebGPU)
endif()

if(RHI_BUILD_EXAMPLES)
    add_compile_definitions(RHI_BUILD_EXAMPLES)
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

