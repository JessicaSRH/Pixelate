#pragma once

#include "vma_usage.h"

namespace PixelateSettings
{
	inline constexpr VkFormat PREFERRED_SWAPCHAIN_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	inline constexpr VkColorSpaceKHR PREFERRED_SWAPCHAIN_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	inline constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
}