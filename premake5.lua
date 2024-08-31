vulkanSDKpath = os.getenv("VULKAN_SDK")
buildDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
pixelateIndlucePath = "Pixelate/Include"
pixelateSourcePath = "Pixelate/Source"

workspace "Pixelate"
	configurations { "Debug", "Debug_Verbose", "Release" }
	platforms { "Win64" }
	startproject "Pixelize"

filter { "platforms:Win64" }
	system "Windows"
	architecture "x86_64"

project "Pixelate"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("build/bin/" .. buildDir .. "/%{prj.name}")
	objdir ("build/obj/" .. buildDir .. "/%{prj.name}")
	
	files {
		pixelateSourcePath .. "/**.h",
		pixelateSourcePath .. "/**.cpp",
		pixelateIndlucePath .. "/**.h",
	}

	includedirs {
		"Pixelate/ThirdParty/spdlog/include",
		"Pixelate/ThirdParty/VulkanProfiles/Include",
		vulkanSDKpath .. "/Include",
		pixelateSourcePath,
		pixelateIndlucePath
	}

	libdirs {
		vulkanSDKpath .. "/Lib"
	}

	links {
		"vulkan-1"
	}

	filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"
	
	filter "configurations:Debug_Verbose"
	defines { "DEBUG", "VERBOSE" }
	symbols "On"

	filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"

	filter "platforms:Win64"
	defines { "WIN64" }

	filter "action:vs*"
		buildoptions { "/MP" }


project "Pixelize"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("build/bin/" .. buildDir .. "/%{prj.name}")
	objdir ("build/obj/" .. buildDir .. "/%{prj.name}")
	
	files {
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp"
	}

	includedirs {
		"Pixelate/ThirdParty/spdlog/include",
		"Pixelate/ThirdParty/VulkanProfiles/Include",
		pixelateIndlucePath,
		vulkanSDKpath .. "/Include"
	}

	libdirs {
		vulkanSDKpath .. "/Lib"
	}

	links {
		"SDL2", "SDL2main", "Pixelate"
	}

	filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"
	
	filter "configurations:Debug_Verbose"
	defines { "DEBUG", "VERBOSE" }
	symbols "On"

	filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"
