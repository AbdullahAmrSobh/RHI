cmake_minimum_required(VERSION 3.19)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

option(TRACY_ENABLE        "Enable Tracy profiler"                          ${PROJECT_IS_TOP_LEVEL})
option(RHI_DEBUG           "Enable RHI debug mode"                          ${PROJECT_IS_TOP_LEVEL})
# option(RHI_SHADER_COMPILER "Builds RHI shader compiler and reflection tool" OFF)
# option(RHI_BUILD_EXAMPLES  "Build examples tree"                            OFF)

# # TODO: REMOVE!!!
# set(RHI_SHADER_COMPILER OFF)
# set(RHI_BUILD_EXAMPLES OFF)

if(NOT EMSCRIPTEN)
    option(RHI_BACKEND_VULKAN "Enable RHI Vulkan 1.3 Backend"  ON)
    option(RHI_BACKEND_D3D12  "Enable RHI DirectX 12 Backend"  OFF)
    option(RHI_BACKEND_WEBGPU "Enable RHI WebGPU Backend"      OFF)
else()
    set(RHI_BACKEND_D3D12  OFF)
    set(RHI_BACKEND_VULKAN OFF)
    set(RHI_BACKEND_WEBGPU ON)
endif()

message(STATUS "TRACY_ENABLE        : ${TRACY_ENABLE}")
message(STATUS "RHI_DEBUG           : ${RHI_DEBUG}")
message(STATUS "RHI_BACKEND_VULKAN  : ${RHI_BACKEND_VULKAN}")
message(STATUS "RHI_BACKEND_D3D12   : ${RHI_BACKEND_D3D12}")
message(STATUS "RHI_BACKEND_WEBGPU  : ${RHI_BACKEND_WEBGPU}")
message(STATUS "RHI_SHADER_COMPILER : ${RHI_SHADER_COMPILER}")
message(STATUS "RHI_BUILD_EXAMPLES  : ${RHI_BUILD_EXAMPLES}")

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

include(cmake/add-target.cmake)
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME           TL
	GIT_REPOSITORY git@github.com:AbdullahAmrSobh/TL.git
	GIT_TAG        main
)

if(RHI_BACKEND_VULKAN)
	CPMAddPackage(
		NAME           RHI_vma
		GIT_REPOSITORY git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
		GIT_TAG        v3.3.0
	)

	CPMAddPackage(
		NAME           RHI_volk
		GIT_REPOSITORY git@github.com:zeux/volk.git
		GIT_TAG        1.4.304
	)
endif()

if(RHI_BACKEND_D3D12)
	CPMAddPackage(
		NAME           RHI_dma
		GIT_REPOSITORY git@github.com:GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git
		GIT_TAG        v2.0.1
	)
endif()

if(RHI_BACKEND_WEBGPU)
	CPMAddPackage(
		NAME           dawn
		GIT_REPOSITORY dawn.googlesource.com/dawn
		GIT_TAG        chromium/7482
		DOWNLOAD_ONLY  YES
		OPTIONS
			"DAWN_ENABLE_D3D11 OFF"
			"DAWN_ENABLE_NULL OFF"
			"DAWN_BUILD_SAMPLES OFF"
			"DAWN_USE_GLFW OFF"
			"DAWN_USE_WAYLAND OFF"
			"DAWN_USE_WINDOWS_UI OFF"
			"DAWN_USE_X11 OFF"
			"TINT_BUILD_TESTS OFF"
	)
endif()

# ---- Declare library ----

add_subdirectory(RHI)

if(WIN32)
    target_compile_definitions(RHI PUBLIC RHI_PLATFORM_WINDOWS)
elseif(ANDROID)
    target_compile_definitions(RHI PUBLIC RHI_PLATFORM_ANDROID)
elseif(APPLE)
    include(TargetConditionals)
    if(TARGET_OS_MAC)
        target_compile_definitions(RHI PUBLIC RHI_PLATFORM_MACOS)
    elseif(TARGET_OS_IPHONE)
        target_compile_definitions(RHI PUBLIC RHI_PLATFORM_IOS)
    endif()
elseif(UNIX)
    target_compile_definitions(RHI PUBLIC RHI_PLATFORM_LINUX)
endif()

if(TRACY_ENABLE)
    target_compile_definitions(RHI PRIVATE TRACY_ENABLE)
endif()

if(RHI_DEBUG)
    target_compile_definitions(RHI PRIVATE RHI_DEBUG)
endif()

if(RHI_BACKEND_D3D12)
    add_subdirectory(D3D12)
endif()

if(RHI_BACKEND_VULKAN)
    add_subdirectory(Vulkan)
endif()

if(RHI_BACKEND_WEBGPU)
    add_subdirectory(WebGPU)
endif()
