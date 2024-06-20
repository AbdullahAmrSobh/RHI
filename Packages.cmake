include(${CMAKE_SOURCE_DIR}/CMake/CPM.cmake)

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
    NAME           RHI_vma
	GIT_REPOSITORY git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
	GIT_TAG        v3.0.1
)

CPMAddPackage(
    NAME           SPIRV-Cross
	GIT_REPOSITORY git@github.com:KhronosGroup/SPIRV-Cross.git
	GIT_TAG        0e2880ab990e79ce6cc8c79c219feda42d98b1e8 # v2024.1.22
	OPTIONS
		"SPIRV_CROSS_ENABLE_TESTS OFF"
		"SPIRV_CROSS_SKIP_INSTALL ON"
		"SPIRV_CROSS_FORCE_PIC ON"
		"BUILD_SHARED_LIBS OFF"
		"SPIRV_CROSS_WERROR OFF"
		"SPIRV_CROSS_CLI OFF"
		"SPIRV_CROSS_ENABLE_CPP OFF"
		"SPIRV_CROSS_ENABLE_MSL OFF"
		"SPIRV_CROSS_ENABLE_C_API OFF"
)

if(RHI_BUILD_EXAMPLES)
	CPMAddPackage(
		NAME           RHI_glm
		GIT_REPOSITORY git@github.com:g-truc/glm.git
		GIT_TAG        0.9.9.8
		OPTIONS
			"GLM_CONFIG_CLIP_CONTROL ON"
			"GLM_CLIP_CONTROL_ZO_BIT ON"
	)
endif()