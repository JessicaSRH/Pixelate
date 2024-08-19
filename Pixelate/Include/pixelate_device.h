#pragma once
#include "internal_pixelate_include.h"

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