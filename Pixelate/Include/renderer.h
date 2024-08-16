#pragma once

#include "resource_manager.h"
#include "pixelate_include.h"

namespace Pixelate
{
	// Swapchain

	constexpr VkFormat PREFERRED_SWAPCHAIN_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	constexpr VkColorSpaceKHR PREFERRED_SWAPCHAIN_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> PresentQueueFamily;
		std::optional<uint32_t> GraphicsQueueFamily;
		std::optional<uint32_t> ComputeQueueFamily;
	};

	struct PixelateDevice
	{
	public:
		VkDevice VkDevice;
		VkPhysicalDevice VkPhysicalDevice;
		QueueFamilyIndices QueueFamilyIndices;
	};

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
		PixelateSwapchain() { }
		PixelateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SDL_Window* window);
		void Dispose();
		void Recreate();
	private :
		void DisposeImageViews();
	private:
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;
		VkSurfaceKHR m_Surface;

	};

	// Presentation Engine

	class PixelatePresentationEngine
	{
	public:
		int GetWidth() { return m_Width; }
		int GetHeight() { return m_Height; }
		const SDL_Window* GetWindow() const { return m_Window; }
		const VkSurfaceKHR GetSurface() const { return m_VkSurfaceKHR; }

	public:
		PixelatePresentationEngine(int width, int height, SDL_Window* window, VkSurfaceKHR surface);
		void InitializeSwapchain(VkDevice device, VkPhysicalDevice physicalDevice);
		void Dispose(VkInstance instance);

	private:
		int m_Width;
		int m_Height;
		SDL_Window* m_Window;
		VkSurfaceKHR m_VkSurfaceKHR;
		PixelateSwapchain m_Swapchain;
	};

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

	// Fences

	class FenceGroup
	{
	public:
		FenceGroup(VkDevice device, uint32_t size);
		void AddFencedCallback(uint32_t subindex = std::numeric_limits<uint32_t>::max());
		void Wait(uint32_t subindex = std::numeric_limits<uint32_t>::max());

	private:
		void Reset(uint32_t subindex = std::numeric_limits<uint32_t>::max());
		std::vector<VkFence> m_VkFences;
		VkDevice m_Device;
	};

	enum class FenceIdenfitier : uint32_t
	{
		FrameInFlight = 1
	};

	class FenceManager
	{
	public:
		FenceManager(VkDevice device);
		FenceGroup& GetFenceGroup(FenceIdenfitier descriptor, uint32_t groupSize = 1);

	private:
		std::unordered_map<FenceIdenfitier, FenceGroup> m_Fences;
		VkDevice m_Device;
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
