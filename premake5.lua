outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
srcdir = "src"
vulkanSDKpath = os.getenv("VULKAN_SDK")

workspace "Pixelate"
	configurations { "Debug", "Release" }
	platforms { "Win64" }
	startproject "Pixelate"
	
filter { "platforms:Win64" }
	system "Windows"
	architecture "x86_64"

project "Pixelate"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("build/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("build/obj/" .. outputdir .. "/%{prj.name}")
	
	files {
		srcdir .. "/**.h",
		srcdir .. "/**.c",
		srcdir .. "/**.cpp"
	}

	includedirs {
		"thirdparty/spdlog/include",
		vulkanSDKpath .. "/Include",
	}

	libdirs {
		vulkanSDKpath .. "/Lib"
	}

	links {
		"SDL2", "SDL2main", "vulkan-1"
	}


	filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"

	filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"