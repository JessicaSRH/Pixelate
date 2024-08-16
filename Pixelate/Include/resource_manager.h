#pragma once

#include "vma_usage.h"

namespace Pixelate
{
	template <typename T>
	using PixelateResourceReference = std::shared_ptr<T>;

	struct ImageViewDescriptor
	{
		std::string Name;
		VkImageViewCreateInfo Descriptor;
	};

	static VmaAllocator CreateVmaAllocator(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t vulkanApiVersion)
	{
		VmaAllocatorCreateInfo vmaCreateInfo{};
		vmaCreateInfo.instance = instance;
		vmaCreateInfo.device = device;
		vmaCreateInfo.physicalDevice = physicalDevice;
		vmaCreateInfo.vulkanApiVersion = vulkanApiVersion;

		VmaAllocator vmaAllocator;
		vmaCreateAllocator(&vmaCreateInfo, &vmaAllocator);

		return vmaAllocator;
	}

	class VulkanResourceManager
	{
	public:
		VulkanResourceManager(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, unsigned int vulkanApiVersion)
			: m_VmaAllocator(CreateVmaAllocator(instance, device, physicalDevice, vulkanApiVersion))
		{}

		VkImageView RequestImageView(ImageViewDescriptor imageViewDescriptor)
		{

		}

		void Dispose() {}

	private:
		VmaAllocator m_VmaAllocator;
		std::map<uint64_t, PixelateResourceReference<VkImageView>> m_ImageViews;
	};

}