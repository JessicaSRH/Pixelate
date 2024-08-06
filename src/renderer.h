#pragma once

#include "pxl8_include.h"

namespace Pixelate
{
	// Todo:
	//  Create vkInstance! [x]
	// 
	//  Load validation layers!
	//  Create vkSurface!
	//  Create vkSwapchain!
	//  Create vkDevice!
	//  Create images, pipeline, framebuffers, renderpasses (dynamic rendering? GRAPH?)!
	//  Create basic shader and compile it into SPIRV
	//  Create render loop!
	//  TRIANGLE!
	// 
	// Client side todo:
	//  controls! SDL events! camera (orbit)!
	//  load gltf models! tiny loader or whatever it is called? reference counting!
	//  ENTT?
	// 
	// Then more core rendering:
	//  RenderGraph!?

	void LogSDLError(std::string msg);

	struct VulkanContext
	{
		VkInstance Instance;

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
		std::unique_ptr<VulkanContext> s_vulkan;

	public:
		void Init();
		void Render();
		void Dispose();
	};

}
