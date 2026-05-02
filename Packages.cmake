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
