#pragma once

#include "internal_pixelate_include.h"

namespace PixelateSettings
{
	constexpr VkFormat PREFERRED_SWAPCHAIN_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	constexpr VkColorSpaceKHR PREFERRED_SWAPCHAIN_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
}

namespace Pixelate
{
	bool SupportsSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkExtensionProperties> availableExtensions);
	QueueFamilyIndices GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface = VK_NULL_HANDLE);
	
	// Swapchain
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class PixelateSwapchain
	{
	public:
		SwapChainSupportDetails SupportDetails;
		VkSurfaceFormatKHR SurfaceFormat;
		VkPresentModeKHR PresentMode;
		VkExtent2D Extent;
		VkSwapchainKHR VkSwapchain;
		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;
	public:
		PixelateSwapchain() {}
		PixelateSwapchain(PixelateDevice device, VkSurfaceKHR surface, SDL_Window* window);
		void Dispose();
		void Recreate();
	private:
		void DisposeImageViews();
	private:
		PixelateDevice m_Device;
		VkSurfaceKHR m_Surface;

	};

	// Presentation engine
	class PixelateSemaphore;

	class PixelatePresentationEngine
	{
	public:
		int GetWidth() { return m_Width; }
		int GetHeight() { return m_Height; }
		const SDL_Window* GetWindow() const { return m_Window; }
		const VkSurfaceKHR GetSurface() const { return m_VkSurfaceKHR; }

	public:
		PixelatePresentationEngine(int width, int height, SDL_Window* window, VkSurfaceKHR surface);
		void Initialize(PixelateDevice device);
		std::tuple<uint32_t, VkImageView> AcquireSwapcahinImage(VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence signalFence = VK_NULL_HANDLE);
		void Present(
			uint32_t swapchainImageIndex,
			PixelateSemaphore waitSemaphore,
			VkImageLayout previousLayout = VK_IMAGE_LAYOUT_UNDEFINED);
		void Dispose(VkInstance instance);

	private:
		int m_Width;
		int m_Height;
		SDL_Window* m_Window;
		VkSurfaceKHR m_VkSurfaceKHR;
		PixelateDevice m_Device;
		PixelateSwapchain m_Swapchain;
		VkQueue m_PresentQueue;

	private:
		void TransitionImageLayout(
			uint32_t swapchainImageIndex,
			PixelateSemaphore signalSemaphore,
			PixelateSemaphore waitSemaphore,
			VkImageLayout newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED);
	private:
		std::vector<VkCommandBuffer> m_ImageTransitionCommandBuffers;
	};
}