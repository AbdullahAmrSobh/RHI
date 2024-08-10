include(${CMAKE_SOURCE_DIR}/CMake/CPM.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/variables.cmake)

# CPMAddPackage(
#     NAME           slang
# 	GIT_REPOSITORY git@github.com:shader-slang/slang.git
# 	GIT_TAG        c00f461aad3d997a2e1c59559421275d6339ae6f # v2024.1.22
# 	OPTIONS
# 		"SLANG_ENABLE_TESTS OFF"
# 		"SLANG_ENABLE_EXAMPLES OFF"
# 		"SLANG_ENABLE_GFX OFF"
# 		"SLANG_ENABLE_CUDA FALSE"
# )

CPMAddPackage(
    NAME           TL
	GIT_REPOSITORY git@github.com:AbdullahAmrSobh/TL.git
	GIT_TAG        main
	OPTIONS
		TL_ENABLE_TRACY ${PROJECT_IS_TOP_LEVEL}
		TL_ENABLE_TRACY_MEMORY_TRACKING ${PROJECT_IS_TOP_LEVEL}
)

CPMAddPackage(
    NAME           RHI_vma
	GIT_REPOSITORY git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
	GIT_TAG        v3.0.1
)

CPMAddPackage(
    NAME           SPIRV-Reflect
	GIT_REPOSITORY git@github.com:KhronosGroup/SPIRV-Reflect.git
	GIT_TAG        756e7b13243b5c4b110bb63dba72d10716dd1dfe
	OPTIONS
		"SPIRV_REFLECT_EXECUTABLE OFF"
		"SPIRV_REFLECT_STATIC_LIB ON"
)

if(RHI_BUILD_EXAMPLES)
	CPMAddPackage(
		NAME           glm
		GIT_REPOSITORY git@github.com:g-truc/glm.git
		GIT_TAG        0.9.9.8
		OPTIONS
			"GLM_CONFIG_CLIP_CONTROL ON"
			"GLM_CLIP_CONTROL_ZO_BIT ON"
	)

	CPMAddPackage(
		NAME           Compressonator
		GIT_REPOSITORY git@github.com:GPUOpen-Tools/compressonator.git
		GIT_TAG        V4.5.52
		GIT_SUBMODULES_RECURSE TRUE
		GIT_PROGRESS TRUE
		OPTIONS
			"OPTION_ENABLE_ALL_APPS OFF"             # Enable all apps
			"OPTION_BUILD_APPS_CMP_CLI OFF"
			"OPTION_BUILD_APPS_CMP_GUI OFF"
			"OPTION_BUILD_CMP_SDK OFF"
			"OPTION_BUILD_APPS_CMP_VISION OFF"
			"OPTION_BUILD_APPS_CMP_UNITTESTS OFF"    # Build Compressontor UnitTests
			"OPTION_BUILD_APPS_CMP_EXAMPLES OFF"     # Build Compressontor Examples
			"OPTION_BUILD_BROTLIG OFF"
			"OPTION_BUILD_KTX2 OFF"
			"LIB_BUILD_CORE ON"
			"LIB_BUILD_COMPRESSONATOR_SDK ON"
			"LIB_BUILD_FRAMEWORK_SDK ON"
			"LIB_BUILD_GPUDECODE ON"
	)

	CPMAddPackage(
		NAME           meshoptimizer
		GIT_REPOSITORY git@github.com:zeux/meshoptimizer.git
		GIT_TAG        v0.21
		OPTIONS
	)

	CPMAddPackage(
		NAME           assimp
		GIT_REPOSITORY git@github.com:assimp/assimp.git
		GIT_TAG        v5.4.2
		OPTIONS
	)

	CPMAddPackage(
		NAME           glfw
		GIT_REPOSITORY git@github.com:glfw/glfw.git
		GIT_TAG        3.4
		OPTIONS
	)
endif()