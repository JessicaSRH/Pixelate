#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <functional>
#include <ranges>

#include "pixelate_log.h"
#include "pixelate_window.h"
#include "pixelate_renderer.h"

#ifdef DEBUG
constexpr bool VALIDATION_LAYERS_ENABLED = true;
#else
constexpr bool VALIDATION_LAYERS_ENABLED = false;
#endif

enum class Platform{ Win64 };

#ifdef WIN64
constexpr Platform PLATFORM = Platform::Win64;
#endif


// Todo:
//  Create vkInstance! [x]
//  Load validation layers! [x]
//  Create VkPhysicalDevice and VkDevice! [x]
//  Create vkSurface! [x]
// 
//  Create vkSwapchain!
//  Create images, pipeline, framebuffers, renderpasses (dynamic rendering? GRAPH?)!
//  Create basic shader and compile it into SPIRV
//  Create render loop!
//  TRIANGLE!
// 
// Ray-tracing!?
// 
// Client side todo:
//  Orbit camera with control through SDL events if possible
//  load gltf models! tiny loader or whatever it is called? reference counting!
//  ENTT?
//  IMGUI?
// 
// Then more core rendering:
//  RenderGraph!?
//
