
#include "renderer.h"

#include "vulkan/vulkan.h"

namespace Pixelate
{
	std::vector<const char*> VectorOfStringsToCPointer(const std::vector<std::string>& strings)
	{
		auto c_strings = std::vector<const char*>(strings.size());

		for (auto x : strings)
			c_strings.push_back(x.c_str());

		return c_strings;
	}

	void LogSDLError(std::string msg = "SDL Error: ")
	{
		auto errorString = msg + SDL_GetError();
		PXL8_CORE_ERROR(errorString);
	}

	static int InitializeSDL()
	{
		int result = SDL_Init(SDL_INIT_EVERYTHING);

		if (result)
		{
			LogSDLError("SDL2 initialization failed: ");
			return result;
		}

		PXL8_CORE_TRACE("SDL initialized successfully.");

		return 0;
	}

	static void ValidateVulkanInstance(VkResult& result)
	{
		switch (result)
		{
			case VK_SUCCESS:
				PXL8_CORE_TRACE("Vulkan instance created successfully.");
				return;
			default:
				PXL8_CORE_ERROR("Failed to create vulkan instance!");
		}
	}

	static VkInstance CreateVulkanInstance(const std::vector<std::string>& layers, const std::vector<std::string>& extensions)
	{
		auto applicationInfo = VkApplicationInfo();
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = "Pixelize";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.pEngineName = "Pixelate";
		applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		auto layer_c_strings = VectorOfStringsToCPointer(layers);
		auto extension_c_strings = VectorOfStringsToCPointer(extensions);

		auto createInfo = VkInstanceCreateInfo();
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layer_c_strings.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extension_c_strings.data();
		createInfo.pNext = 0;

		VkInstance instance;
		auto result = vkCreateInstance(&createInfo, 0, &instance);

		ValidateVulkanInstance(result);

		return instance;
	}

	static void DisposeWindow(SDL_Window*& window)
	{
		if (!window) // nothing to dispose
			return;

		SDL_DestroyWindow(window);
		window = 0;

		PXL8_CORE_TRACE("SDL window disposed successfully.");
	}

	void Renderer::Init()
	{
		InitializeSDL();

		s_window = SDL_CreateWindow("Pixelate", 150, 150, 800, 600, SDL_WINDOW_SHOWN);

		if (s_window)
			PXL8_CORE_TRACE("SDL window created successfully.");
		else
			LogSDLError("SDL window creation failed: ");

		auto instanceLayers = std::vector<std::string>{};
		auto instanceExtensions = std::vector<std::string>{};

		s_vulkan = std::make_unique<VulkanContext>();

		s_vulkan->Instance = CreateVulkanInstance(instanceLayers, instanceExtensions);

		SDL_Delay(3000);
	}

	void Renderer::Dispose()
	{
		s_vulkan->Dispose();
		DisposeWindow(s_window);
		SDL_Quit();

		PXL8_CORE_TRACE("Renderer disposed successfully.");
	}

	// --------------------------------------------------------------
	// VulkanContext
	//--------------------------------------------------------------

	void VulkanContext::Dispose()
	{
		if (s_isDisposed)
			return;

		vkDestroyInstance(Instance, 0);
		s_isDisposed = true;
		PXL8_CORE_TRACE("VkInstance destroyed successfully.");
	}

	VulkanContext::~VulkanContext()
	{
		Dispose();
	}
}
