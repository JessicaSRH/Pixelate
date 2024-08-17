#pragma once

#include "resource_manager.h"
#include "pixelate_include.h"

namespace PixelateSettings
{
	constexpr VkFormat PREFERRED_SWAPCHAIN_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	constexpr VkColorSpaceKHR PREFERRED_SWAPCHAIN_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
}

namespace Pixelate
{
	// Device

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
}
#include "command_buffer_manager.h"
namespace Pixelate {
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
		PixelateSwapchain() { }
		PixelateSwapchain(PixelateDevice device, VkSurfaceKHR surface, SDL_Window* window);
		void Dispose();
		void Recreate();
	private :
		void DisposeImageViews();
	private:
		PixelateDevice m_Device;
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
		void Initialize(PixelateDevice device);
		std::tuple<VkImage, VkImageView> AcquireSwapcahinImage(VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence signalFence = VK_NULL_HANDLE);
		void Present(uint32_t index, VkSemaphore waitSemaphore = VK_NULL_HANDLE, VkImageLayout previousLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkFence signalFence = VK_NULL_HANDLE);
		void Dispose(VkInstance instance);

	private:
		int m_Width;
		int m_Height;
		SDL_Window* m_Window;
		VkSurfaceKHR m_VkSurfaceKHR;
		PixelateDevice m_Device;
		PixelateSwapchain m_Swapchain;
		VkQueue m_PresentQueue;
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

	enum class FenceIdenfitier : uint32_t
	{
		FrameHasBeenPresented = 1
	};

	struct FenceGroupDescriptor
	{
		FenceIdenfitier Identifier = FenceIdenfitier::FrameHasBeenPresented;
		uint32_t FenceCount = 1;
		VkFenceCreateFlags CreateFlags = VK_FENCE_CREATE_SIGNALED_BIT;

		uint64_t Hash() const;
	};

	class FenceGroup
	{
	public:
		FenceGroup(VkDevice device, const FenceGroupDescriptor& descriptor);
		void AddFencedCallback(std::function<void()> callback, uint32_t index = std::numeric_limits<uint32_t>::max());
		void Wait(uint32_t index = std::numeric_limits<uint32_t>::max(), uint64_t timeout = std::numeric_limits<uint64_t>::max());
		void Dispose();

	private:
		std::vector<VkFence> m_VkFences;
		std::unordered_map<uint32_t, std::vector<std::function<void()>>> m_IndexedCallbacks;
		VkDevice m_Device;
	};

	class FenceManager
	{
	public:
		FenceManager(VkDevice device);
		FenceGroup& GetFenceGroup(const FenceGroupDescriptor& descriptor);
		void Dispose();

	private:
		std::unordered_map<uint64_t, FenceGroup> m_FenceGroups;
		VkDevice m_Device;
	};

	// Semaphores

	enum class SemaphoreIdentifier : uint32_t
	{
		ImageHasBeenAcquired
	};

	struct SemaphoreGroupDescriptor
	{
		SemaphoreIdentifier Identifier = SemaphoreIdentifier::ImageHasBeenAcquired;
		uint32_t SemaphoreCount = 1;

		uint64_t Hash() const;
	};

	class SemaphoreManager
	{
	public:
		SemaphoreManager(VkDevice device);
		std::vector<VkSemaphore>& GetSemaphoreGroup(const SemaphoreGroupDescriptor& descriptor);
		void Dispose();

	private:
		std::unordered_map<uint64_t, std::vector<VkSemaphore>> m_SemaphoreGroups;
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
		FenceManager m_FenceManager;
		SemaphoreManager m_SemaphoreManager;
		uint32_t m_FrameInFlightIndex = 0;
	};
}
