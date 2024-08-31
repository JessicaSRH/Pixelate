#pragma once

// Define macros that need to be defined before vulkan include here, as per https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html

#include <vma/vk_mem_alloc.h>
#define VP_USE_OBJECT
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "vulkan_profiles.hpp"

namespace PixelateVulkanProfile
{
	//constexpr const char* PROFILE_NAME = PIXELATE_ENGINE_BASELINE_PROFILE_NAME;
	//constexpr uint32_t PROFILE_SPEC_VERSION = PIXELATE_ENGINE_BASELINE_PROFILE_SPEC_VERSION;
	//constexpr uint32_t PROFILE_MIN_API_VERSION = PIXELATE_ENGINE_BASELINE_PROFILE_MIN_API_VERSION;

	constexpr const char* PROFILE_NAME = VP_PIXELATE_BASELINE_PROFILE_NAME;
	constexpr uint32_t PROFILE_SPEC_VERSION = VP_PIXELATE_BASELINE_PROFILE_SPEC_VERSION;
	constexpr uint32_t PROFILE_MIN_API_VERSION = VP_PIXELATE_BASELINE_PROFILE_MIN_API_VERSION;

}
