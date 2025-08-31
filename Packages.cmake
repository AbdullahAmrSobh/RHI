include(${CMAKE_SOURCE_DIR}/CMake/CPM.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/variables.cmake)

CPMAddPackage(
  NAME             slang
	GIT_REPOSITORY git@github.com:shader-slang/slang.git
	GIT_TAG        v2025.15.1
	OPTIONS
		"SLANG_EMBED_CORE_MODULE_SOURCE ON"
		"SLANG_EMBED_CORE_MODULE ON"
		"SLANG_ENABLE_DXIL OFF"
		"SLANG_ENABLE_FULL_IR_VALIDATION OFF"
		"SLANG_ENABLE_IR_BREAK_ALLOC OFF"
		"SLANG_ENABLE_ASAN OFF"
		"SLANG_ENABLE_PREBUILT_BINARIES OFF"
		"SLANG_ENABLE_GFX OFF"
		"SLANG_ENABLE_SLANGD OFF"
		"SLANG_ENABLE_SLANGC OFF"
		"SLANG_ENABLE_SLANGI OFF"
		"SLANG_ENABLE_SLANGRT OFF"
		"SLANG_ENABLE_TESTS OFF"
		"SLANG_ENABLE_EXAMPLES OFF"
		"SLANG_ENABLE_REPLAYER OFF"
		"SLANG_SPIRV_HEADERS_INCLUDE_DIR OFF"
		"SLANG_ENABLE_RELEASE_DEBUG_INFO OFF"
		"SLANG_ENABLE_RELEASE_LTO OFF"
		"SLANG_ENABLE_SPLIT_DEBUG_INFO OFF"
		"SLANG_ENABLE_SLANG_RHI OFF"
		"SLANG_RHI_ENABLE_AGILITY_SDK OFF"
		"SLANG_RHI_ENABLE_CPU OFF"
		"SLANG_RHI_ENABLE_D3D11 OFF"
		"SLANG_RHI_ENABLE_D3D12 OFF"
		"SLANG_RHI_ENABLE_NVAPI OFF"
		"SLANG_RHI_ENABLE_OPTIX OFF"
		"SLANG_RHI_ENABLE_VULKAN OFF"
		"SLANG_RHI_ENABLE_WGPU OFF"
)

CPMAddPackage(
    NAME           TL
	GIT_REPOSITORY git@github.com:AbdullahAmrSobh/TL.git
	GIT_TAG        main
	OPTIONS
		TL_ENABLE_TRACY ${PROJECT_IS_TOP_LEVEL}
		TL_ENABLE_TRACY_MEMORY_TRACKING ${PROJECT_IS_TOP_LEVEL}
)

if(RHI_BACKEND_VULKAN)
	CPMAddPackage(
		NAME           RHI_vma
		GIT_REPOSITORY git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
		GIT_TAG        v3.3.0
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
		GIT_REPOSITORY git@github.com:google/dawn.git
		GIT_TAG        46b4670bc67cb4f6d34f6ce6a46ba7e1d6059abf # chromium/7258
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

if(RHI_BUILD_EXAMPLES)
	if (NOT EMSCRIPTEN)
		CPMAddPackage(
			NAME           glfw
			GIT_REPOSITORY git@github.com:glfw/glfw.git
			GIT_TAG        3.4
			OPTIONS
		)
	endif()

	CPMAddPackage(
		NAME           imgui
		GIT_REPOSITORY git@github.com:ocornut/imgui.git
		GIT_TAG        v1.91.9b-docking
		DOWNLOAD_ONLY  YES
	)

	if (imgui_ADDED)
		set(SOURCE_FILES
			${imgui_SOURCE_DIR}/imgui.cpp
			${imgui_SOURCE_DIR}/imgui_widgets.cpp
			${imgui_SOURCE_DIR}/imgui_tables.cpp
			${imgui_SOURCE_DIR}/imgui_demo.cpp
			${imgui_SOURCE_DIR}/imgui_draw.cpp
		)
		set(HEADER_FILES
			${imgui_SOURCE_DIR}/imstb_truetype.h
			${imgui_SOURCE_DIR}/imstb_textedit.h
			${imgui_SOURCE_DIR}/imstb_rectpack.h
			${imgui_SOURCE_DIR}/imgui.h
			${imgui_SOURCE_DIR}/imgui_internal.h
			${imgui_SOURCE_DIR}/imconfig.h
		)
		if(NOT TARGET TL)
			include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
		else()
			include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
		endif()
		tl_add_target(
			NAME rhi_imgui
			# NAMESPACE Thirdparty
			STATIC
			HEADERS ${HEADER_FILES}
			SOURCES ${SOURCE_FILES}
		)
		target_include_directories(rhi_imgui ${warning_guard} PUBLIC ${imgui_SOURCE_DIR})
		# Set compiler flags for your target
		target_compile_options(rhi_imgui PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W3>" "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall;-Wextra>")
	endif()

	CPMAddPackage(
		NAME           glm
		GIT_REPOSITORY git@github.com:g-truc/glm.git
		GIT_TAG        1.0.1
		OPTIONS
			"GLM_CONFIG_CLIP_CONTROL ON"
			"GLM_CLIP_CONTROL_ZO_BIT ON"
	)

	CPMAddPackage(
		NAME           assimp
		GIT_REPOSITORY git@github.com:assimp/assimp.git
		GIT_TAG        v6.0.2
		OPTIONS
			ASSIMP_WARNINGS_AS_ERRORS OFF
			ASSIMP_BUILD_TESTS OFF
	)
	set_target_properties(assimp PROPERTIES EXCLUDE_FROM_ALL TRUE)

endif()