#pragma once

// Todo:
//  Create vkInstance! [x]
//  Load validation layers! [x]
//  Create VkPhysicalDevice and VkDevice! [x]
//  Create vkSurface! [x]
//  Create vkSwapchain! [x]
//  Create render loop + presentation[x]
//  
//  Pipeline manager [ ]
//  Framebuffer manager [ ]
//  Render passes / render graph (dynamic rendering?) [ ]
//  Create basic triangle shader and compile it into SPIRV [ ]
//  TRIANGLE! [ ]
// 
//  Resource (image/buffer) manager [ ]
// 
// Client side todo:
//  Orbit camera with control through SDL events if possible
//  load gltf models! tiny loader or whatever it is called? reference counting!
//  ENTT?
//  IMGUI?
// 
// Ray-tracing!?
// 
// Then more core rendering:
//  RenderGraph!?


// SUPPRESS WARNINGS FROM EXTERNAL LIBRARIES
#ifdef _MSC_VER
#pragma warning(push, 0)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#endif

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <functional>
#include <ranges>
#include <memory>
#include <limits>
#include <tuple>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "vma_usage.h" // includes Vulkan and SDL2

#include "log.h"

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

// END SUPPRESS WARNINGS FROM EXTERNAL LIBRARIES

#ifndef PIXELATE_SETTINGS_DEFINED
#define PIXELATE_SETTINGS_DEFINED
namespace PixelateSettings
{
	inline constexpr VkFormat PREFERRED_SWAPCHAIN_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	inline constexpr VkColorSpaceKHR PREFERRED_SWAPCHAIN_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	inline constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
}
#endif

#include "pixelate_helpers.h"
#include "thread_safe_fifo_queue.h"
#include "hasher.h"
#include "window.h"
#include "pixelate_device.h"
#include "semaphore_manager.h"
#include "fence_manager.h"
#include "command_buffer_manager.h"
#include "queue_manager.h"
#include "resource_manager.h"
#include "presentation_engine.h"
#include "pipeline_manager.h"

#ifndef VALIDATION_DEFINITIONS
#define VALIDATION_DEFINITIONS
#ifdef DEBUG
inline constexpr bool VALIDATION_LAYERS_ENABLED = true;
#else
inline constexpr bool VALIDATION_LAYERS_ENABLED = false;
#endif

enum class Platform { Win64 };

#ifdef WIN64
inline constexpr Platform PLATFORM = Platform::Win64;
#endif
#endif

