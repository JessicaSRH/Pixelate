#pragma once

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

#include "log.h"
#include "renderer.h"

// Todo:
//  Create vkInstance! [x]
//  Load validation layers! [x]
//  Create VkPhysicalDevice and VkDevice! [x]
//  Create vkSurface! [x]
//  Create vkSwapchain! [x]
// 
//  Create render loop + presentation
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
