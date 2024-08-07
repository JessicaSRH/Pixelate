#pragma once

#include "pxl8_include.h"

namespace Pixelate
{
	// Todo:
	//  Create vkInstance! [x]
	//  Load validation layers! [x]
	// 
	//  Create VkPhysicalDevice and VkDevice!
	//  Create vkSurface!
	//  Create vkSwapchain!
	//  Create images, pipeline, framebuffers, renderpasses (dynamic rendering? GRAPH?)!
	//  Create basic shader and compile it into SPIRV
	//  Create render loop!
	//  TRIANGLE!
	// 
	// Client side todo:
	//  Orbit camera with control through SDL events if possible
	//  load gltf models! tiny loader or whatever it is called? reference counting!
	//  ENTT?
	// 
	// Then more core rendering:
	//  RenderGraph!?

	struct VulkanContext
	{
		VkInstance Instance;
		VkDebugUtilsMessengerEXT DebugMessenger;

		void Dispose();

		~VulkanContext();

		inline bool IsDisposed()
		{
			return s_isDisposed;
		};

	private:
		bool s_isDisposed;
	};

	class Renderer
	{
	private:

		SDL_Window* s_window;
		VulkanContext s_vulkan;

	public:
		void Init();
		void Render();
		void Dispose();
	};

}
