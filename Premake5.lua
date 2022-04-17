project "RHI"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	-- staticruntime "on"
	exceptionhandling "Off"
	
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"Private/**.hpp",
		"Private/**.cpp",
		"Public/**.hpp",
		"Public/**.cpp"
	}
	
	includedirs
	{
		"Public",
		"Private",
		"%{VULKAN_SDK}/include",
	}
	
	links
	{
		"%{VULKAN_SDK}/Lib/vulkan-1.lib"
	}
	
	filter "system:windows"
		systemversion "latest"
	
	filter "configurations:Debug"
		defines "RHI_DEBUG"
		runtime "Debug"
		symbols "on"
	
	filter "configurations:Release"
		defines "RHI_RELEASE"
		runtime "Release"
		optimize "on"
	
	filter "configurations:Dist"
		defines "RHI_DIST"
		runtime "Release"
		optimize "on"
