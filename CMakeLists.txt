cmake_minimum_required(VERSION 3.19)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(
    RHI
    VERSION 0.1.0
    DESCRIPTION "Renderer Hardware Interface"
    HOMEPAGE_URL "https://github.com/AbdullahAmrSobh/RHI"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TRACY_ENABLE "Enable Tracy profiler" ${PROJECT_IS_TOP_LEVEL})
option(RHI_DEBUG "Enable RHI debug mode" ${PROJECT_IS_TOP_LEVEL})
if(NOT EMSCRIPTEN)
    option(RHI_BACKEND_VULKAN "Enable RHI Vulkan 1.3 Backend" ON)
    option(RHI_BACKEND_D3D12 "Enable RHI DirectX 12 Backend" OFF)
    option(RHI_BACKEND_WEBGPU "Enable RHI WebGPU Backend" OFF)
    option(RHI_BACKEND_DYNAMIC "Enable RHI dynamic runtime dispatch backend" ON)
else()
	set(RHI_BACKEND_WEBGPU ON)
    set(RHI_BACKEND_D3D12 OFF)
    set(RHI_BACKEND_VULKAN OFF)
endif()

message(STATUS "TRACY_ENABLE        : ${TRACY_ENABLE}")
message(STATUS "RHI_DEBUG           : ${RHI_DEBUG}")
message(STATUS "RHI_BACKEND_VULKAN  : ${RHI_BACKEND_VULKAN}")
message(STATUS "RHI_BACKEND_D3D12   : ${RHI_BACKEND_D3D12}")
message(STATUS "RHI_BACKEND_WEBGPU  : ${RHI_BACKEND_WEBGPU}")
message(STATUS "RHI_BACKEND_DYNAMIC : ${RHI_BACKEND_DYNAMIC}")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Please read the BUILDING document before trying to build this project.")
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
		GIT_REPOSITORY https://dawn.googlesource.com/dawn
		GIT_TAG        chromium/7827
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

add_subdirectory(RHI)
