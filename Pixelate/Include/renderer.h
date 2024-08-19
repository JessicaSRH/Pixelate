#pragma once

#include "internal_pixelate_include.h"


namespace Pixelate {

	// Instance

	constexpr const char* VK_KHR_win32_surface_extension = "VK_KHR_win32_surface";
	constexpr const char* VK_KHR_surface_extension = "VK_KHR_surface";

	struct PixelateInstance
	{
	public:
		VkInstance Instance;
		VpProfileProperties ProfileProperties;
		VpCapabilities ProfileCapabilities;
		VkDebugUtilsMessengerEXT DebugMessenger;
	};

	// Renderer
	class Renderer
	{
	public:
		Renderer(const char* applicationName, int x, int y, int width, int height, const char* vulkanProfileName = VP_KHR_ROADMAP_2022_NAME, const int profileSpecVersion = VP_KHR_ROADMAP_2022_SPEC_VERSION, unsigned int minApiVersion = VP_KHR_ROADMAP_2022_MIN_API_VERSION);

		const VulkanResourceManager& GetImageManager() const { m_VulkanResourceManager; }

		void Render(std::function<bool()> inputHandler);
		const SDL_Window* GetWindow() const;

		~Renderer();

	private:
		PixelateInstance m_Instance;
		PixelatePresentationEngine m_Presentation;
		PixelateDevice m_Device;
		VulkanResourceManager m_VulkanResourceManager;
		uint32_t m_FrameInFlightIndex = 0;
	};
}
