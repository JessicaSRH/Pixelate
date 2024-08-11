#pragma once

#include "pixelate_include.h"
#include "pixelate_window.h"

#include "vulkan/vulkan.h"
#define VP_USE_OBJECT
#include <vulkan/vulkan_profiles.hpp>

namespace Pixelate
{
	constexpr const char* VK_KHR_win32_surface_extension = "VK_KHR_win32_surface";
	constexpr const char* VK_KHR_surface_extension = "VK_KHR_surface";

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> PresentQueueFamily;
		std::optional<uint32_t> GraphicsQueueFamily;
		std::optional<uint32_t> ComputeQueueFamily;
	};

	class Device
	{
	public:
		VkDevice VkDevice;
		VkPhysicalDevice VkPhysicalDevice;
		QueueFamilyIndices QueueFamilyIndices;
	};

	struct VulkanContext
	{
	public:
		VkInstance Instance;
		VkDebugUtilsMessengerEXT DebugMessenger;
		Device Device;
	};

	struct PixelateSurface
	{
	public:
		uint32_t Width;
		uint32_t Height;
		SDL_Window* Window;
		VkSurfaceKHR VkSurfaceKHR;
	};

	class Renderer
	{
	public:
		Renderer(const char* applicationName, int x, int y, int width, int height, const char* vulkanProfileName = VP_KHR_ROADMAP_2022_NAME, const int profileSpecVersion = VP_KHR_ROADMAP_2022_SPEC_VERSION, unsigned int minApiVersion = VP_KHR_ROADMAP_2022_MIN_API_VERSION);
		~Renderer();

	private:
		VulkanContext m_Vulkan;
		PixelateSurface m_Surface;
	};
}
