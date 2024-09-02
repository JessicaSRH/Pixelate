#include "presentation_engine.h"
#include "internal_pixelate_include.h"
#include "log.h"
#include "semaphore_manager.h"
#include "queue_manager.h"
#include "fence_manager.h"
#include "command_buffer_manager.h"
#include "window.h"
#include "pixelate_settings.h"

namespace Pixelate
{
	static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == PixelateSettings::PREFERRED_SWAPCHAIN_IMAGE_FORMAT
				&& availableFormat.colorSpace == PixelateSettings::PREFERRED_SWAPCHAIN_COLOR_SPACE)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	static VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR; // support guaranteed
	}

	static VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& Capabilities, SDL_Window* window)
	{
		if (Capabilities.currentExtent.width != UINT32_MAX)
		{
			return Capabilities.currentExtent;
		}
		else
		{
			int width, height;
			SDL_Vulkan_GetDrawableSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(Capabilities.minImageExtent.width, std::min(Capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(Capabilities.minImageExtent.height, std::min(Capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	static void ValidateSwapchainCreation(VkResult result)
	{
		switch (result)
		{
		case VK_SUCCESS:
			PXL8_CORE_TRACE("Swapchain created succesfully.");
			break;
		case VK_SUBOPTIMAL_KHR:
			PXL8_CORE_WARN("Swapchain is suboptimal.");
			break;
		default:
			PXL8_CORE_ERROR("Failed to create swapchain.");
		}
	}

	QueueFamilyIndices GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices result{};

		auto i = 0;
		for (const auto& queueFamilyProperties : queueFamilies)
		{
			if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				result.GraphicsQueueFamily = i;

			if (queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
				result.ComputeQueueFamily = i;

			if (surface != VK_NULL_HANDLE && !result.PresentQueueFamily.has_value())
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

				if (presentSupport == VK_TRUE)
					result.PresentQueueFamily = i;
			}

			i++;
		}

		return result;
	}

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.Formats.data());
		}
		else
		{
			PXL8_CORE_ERROR("No suported surface formats found on physical device!");
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.PresentModes.data());
		}
		return details;
	}

	bool SupportsSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkExtensionProperties> availableExtensions)
	{
		auto swapchainExtensionSupported = false;
		for (const auto& extension : availableExtensions)
			if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				swapchainExtensionSupported = true;

		bool swapchainAdequate = false;
		if (swapchainExtensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
			swapchainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

		return swapchainExtensionSupported && swapchainAdequate;
	}

	PixelateSwapchain::PixelateSwapchain(PixelateDevice device, VkSurfaceKHR surface, SDL_Window* window) :
		SupportDetails(QuerySwapChainSupport(device.VkPhysicalDevice, surface)),
		SurfaceFormat(ChooseSwapchainSurfaceFormat(SupportDetails.Formats)),
		PresentMode(ChooseSwapchainPresentMode(SupportDetails.PresentModes)),
		Extent(ChooseSwapchainExtent(SupportDetails.Capabilities, window)),
		VkSwapchain(VK_NULL_HANDLE),
		m_Device(device),
		m_Surface(surface)
	{
		Recreate();
	}

	void PixelateSwapchain::Dispose()
	{
		DisposeImageViews();
		vkDestroySwapchainKHR(m_Device.VkDevice, VkSwapchain, nullptr);
	}

	void PixelateSwapchain::DisposeImageViews()
	{
		for (auto imageView : SwapchainImageViews)
			vkDestroyImageView(m_Device.VkDevice, imageView, nullptr);

		SwapchainImageViews.clear();
	}

	void PixelateSwapchain::Recreate()
	{
		DisposeImageViews();

		uint32_t imageCount = std::clamp((uint32_t)3, (uint32_t)SupportDetails.Capabilities.minImageCount, (uint32_t)SupportDetails.Capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = m_Surface;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = SurfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent = Extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.preTransform = SupportDetails.Capabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = PresentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VkSwapchain;

		QueueFamilyIndices indices = GetQueueFamilyIndices(m_Device.VkPhysicalDevice, m_Surface);
		uint32_t queueFamilyIndices[] = { indices.GraphicsQueueFamily.value(), indices.PresentQueueFamily.value() };

		if (indices.GraphicsQueueFamily != indices.PresentQueueFamily)
		{
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
			swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		ValidateSwapchainCreation(vkCreateSwapchainKHR(m_Device.VkDevice, &swapchainCreateInfo, nullptr, &VkSwapchain));

		uint32_t actualImageCount{};
		vkGetSwapchainImagesKHR(m_Device.VkDevice, VkSwapchain, &actualImageCount, nullptr);

		SwapchainImages.resize(actualImageCount);
		vkGetSwapchainImagesKHR(m_Device.VkDevice, VkSwapchain, &actualImageCount, SwapchainImages.data());

		SwapchainImageViews.resize(SwapchainImages.size());
		for (auto i = 0; i < actualImageCount; i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = SurfaceFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.image = SwapchainImages[0];

			if (vkCreateImageView(m_Device.VkDevice, &createInfo, nullptr, &SwapchainImageViews[i]) != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to create swapchain image views!");
			else
				PXL8_CORE_TRACE(std::string("Swapchain image view ") + std::to_string(i) + " created successfully.");
		}
	}

	PixelatePresentationEngine::PixelatePresentationEngine(int width, int height, SDL_Window* window, VkSurfaceKHR surface)
		: m_Width(width), m_Height(height), m_Window(window), m_VkSurfaceKHR(surface), m_Swapchain()
	{}

	void PixelatePresentationEngine::Initialize(PixelateDevice device)
	{
		m_Device = device;

		vkGetDeviceQueue(m_Device.VkDevice, m_Device.QueueFamilyIndices.PresentQueueFamily.value(), 0, &m_PresentQueue);;

		m_Swapchain = PixelateSwapchain(device, m_VkSurfaceKHR, m_Window);

		m_ImageTransitionCommandBuffers.resize(m_Swapchain.SwapchainImages.size());
	}

	uint32_t PixelatePresentationEngine::AcquireSwapcahinImage(VkSemaphore signalSemaphore, VkFence signalFence)
	{
		uint32_t imageIndex{};
		auto result = vkAcquireNextImageKHR(m_Device.VkDevice, m_Swapchain.VkSwapchain, std::numeric_limits<uint64_t>::max(), signalSemaphore, signalFence, &imageIndex);

		return { imageIndex };
	}

	void ValidateSwapchainResult(VkResult result)
	{
		if (result == VK_SUBOPTIMAL_KHR)
			PXL8_APP_WARN("Swapchain is suboptimal!");

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			PXL8_APP_WARN("Swapchain is out of date!");

		if (result == VK_ERROR_DEVICE_LOST)
			PXL8_APP_WARN("Device was lost during swapchain presentation!");

		if (result != VK_SUCCESS)
			PXL8_APP_WARN("Unknown error during present queue submission!");
	}

	void PixelatePresentationEngine::TransitionImageLayout(
		uint32_t swapchainImageIndex,
		VkSemaphoreSubmitInfo* pSignalSemaphore,
		uint32_t signalSemaphoreCount,
		VkSemaphoreSubmitInfo* pWaitSemaphore,
		uint32_t waitSemaphoreCount,
		VkImageLayout newLayout,
		VkImageLayout oldLayout)
	{
		// Don't re-record the command buffer, it'll be the same every frame
		if (m_ImageTransitionCommandBuffers[swapchainImageIndex] == VK_NULL_HANDLE)
		{
			m_ImageTransitionCommandBuffers[swapchainImageIndex] = CommandBufferManager::GetCommandBuffer(m_Device);

			VkImageMemoryBarrier2 barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_Swapchain.SwapchainImages[swapchainImageIndex];
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
			barrier.srcAccessMask = 0;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
			barrier.dstAccessMask = 0;

			VkDependencyInfo dependencyInfo{};
			dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			dependencyInfo.imageMemoryBarrierCount = 1;
			dependencyInfo.pImageMemoryBarriers = &barrier;

			VkCommandBufferBeginInfo commandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(m_ImageTransitionCommandBuffers[swapchainImageIndex], &commandBufferBeginInfo);
			vkCmdPipelineBarrier2(m_ImageTransitionCommandBuffers[swapchainImageIndex], &dependencyInfo);
			vkEndCommandBuffer(m_ImageTransitionCommandBuffers[swapchainImageIndex]);
		}

		QueueManager::GraphicsQueueSubmit(
			m_Device,
			GraphicsQueueSubmitDescriptor{ GraphicsQueueType::Default },
			m_ImageTransitionCommandBuffers[swapchainImageIndex],
			VK_NULL_HANDLE,
			pSignalSemaphore, signalSemaphoreCount,
			pWaitSemaphore, waitSemaphoreCount
		);
	}

	void PixelatePresentationEngine::Present(
		uint32_t swapchainImageIndex,
		PixelateSemaphore acquireSwapchainImageSemaphore,
		PixelateSemaphore swapchainImageReadyToPresentSemaphore,
		VkImageLayout previousLayout)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain.VkSwapchain;
		presentInfo.pImageIndices = &swapchainImageIndex;

		std::vector<VkSemaphore> waitSemaphores{};

		if (!(previousLayout & (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR | VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)))
		{
			auto& imageTransitionSemaphore = SemaphoreManager::GetSemaphore(
				m_Device.VkDevice,
				VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
				SemaphoreDescriptor
				{
					.Identifier = SemaphoreIdentifier::SwapchainImageTransitionToPresent,
					.Index = swapchainImageIndex
				});
			TransitionImageLayout(
				swapchainImageIndex,
				&imageTransitionSemaphore.SemaphoreSubmitInfo, 1,
				&swapchainImageReadyToPresentSemaphore.SemaphoreSubmitInfo, 1);
			waitSemaphores.push_back(imageTransitionSemaphore);
		}
		else
		{
			waitSemaphores.push_back(swapchainImageReadyToPresentSemaphore);
		}

		presentInfo.waitSemaphoreCount = waitSemaphores.size();
		presentInfo.pWaitSemaphores = waitSemaphores.data();

		ValidateSwapchainResult(vkQueuePresentKHR(m_PresentQueue, &presentInfo));
	}

	void PixelatePresentationEngine::Dispose(VkInstance instance)
	{
		m_Swapchain.Dispose();
		vkDestroySurfaceKHR(instance, m_VkSurfaceKHR, nullptr);
		DestroySDLWindow(m_Window);
	}
}