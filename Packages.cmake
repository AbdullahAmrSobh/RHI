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
    NAME           SPIRV-Reflect
	GIT_REPOSITORY git@github.com:KhronosGroup/SPIRV-Reflect.git
	GIT_TAG        756e7b13243b5c4b110bb63dba72d10716dd1dfe
	OPTIONS
		"SPIRV_REFLECT_EXECUTABLE OFF"
		"SPIRV_REFLECT_STATIC_LIB ON"
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