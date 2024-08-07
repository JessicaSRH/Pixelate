
#include "renderer.h"

#include "vulkan/vulkan.h"

namespace Pixelate
{
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
				PXL8_CORE_INFO("Vulkan instance created successfully.");
				return;
			default:
				PXL8_CORE_ERROR("Failed to create vulkan instance!");
		}
	}

	static bool CheckInstanceLayerSupport(std::vector<const char*>& layers)
	{
		uint32_t propertyCount{};
		vkEnumerateInstanceLayerProperties(&propertyCount, 0); // get count

		std::vector<VkLayerProperties> layerProperties;
		layerProperties.resize(propertyCount);
		vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data()); // get properties

		std::unordered_map<const char*, bool> layerSupportMap;

		for (auto& requiredLayerName : layers)
		{
			layerSupportMap.insert(std::make_pair(requiredLayerName, false));

			for (auto& layerProperty : layerProperties)
				if (strcmp(layerProperty.layerName, requiredLayerName) == 0)
					layerSupportMap.at(requiredLayerName) = true;
		}

		std::vector<std::optional<const char*>> unsupportedLayers{};

		PXL8_CORE_TRACE("Loading Vulkan instance layers...");

		std::transform(layerSupportMap.begin(), layerSupportMap.end(), std::back_inserter(unsupportedLayers),
			[](const auto& kvp) -> std::optional<const char*> {
				if (kvp.second == false)
				{
					PXL8_CORE_WARN(std::string("    Layer not supported: ") + kvp.first);
					return kvp.first;
				}

				PXL8_CORE_TRACE(std::string("    Layer supported: ") + kvp.first);
				return std::nullopt;
			});

		auto allSupported = std::all_of(unsupportedLayers.begin(), unsupportedLayers.end(),
			[](std::optional<const char*> layer) { return layer.has_value(); });

		return allSupported;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LOG_VULKAN_VALIDATION_TRACE(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG_VULKAN_VALIDATION_INFO(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_VULKAN_VALIDATION_WARN(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		default:
			LOG_VULKAN_VALIDATION_ERROR(pCallbackData->pMessage);
		}

		//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfoStruct() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = ValidationLayerCallback;
		return createInfo;
	}

	static void CreateDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT* debugMessenger)
	{
		if (!VALIDATION_LAYERS_ENABLED)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfoStruct();

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debugMessenger) != VK_SUCCESS)
			PXL8_CORE_ERROR("Failed to set up debug messenger!");

		PXL8_CORE_TRACE("Validation layer debug messenger setup successfully.");
	}

	void DestroyDebugUtilsMessengerEXT(const VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VkInstance CreateVulkanInstance(VkInstance& instance)
	{
		auto applicationInfo = VkApplicationInfo();
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = "Pixelize";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.pEngineName = "Pixelate";
		applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		std::vector<const char*> layers{};
		std::vector<const char*> extensions{};

		if (VALIDATION_LAYERS_ENABLED)
		{
			layers.emplace_back("VK_LAYER_KHRONOS_validation");
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		CheckInstanceLayerSupport(layers);

		auto createInfo = VkInstanceCreateInfo();
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfoStruct;
		if (VALIDATION_LAYERS_ENABLED)
		{
			debugMessengerInfoStruct = CreateDebugMessengerCreateInfoStruct();
			createInfo.pNext = &debugMessengerInfoStruct;
		}

		auto result = vkCreateInstance(&createInfo, 0, &instance);

		ValidateVulkanInstance(result);

		return instance;
	}

	static void DestroyWindow(SDL_Window*& window)
	{
		if (!window) // nothing to dispose
			return;

		SDL_DestroyWindow(window);
		window = 0;

		PXL8_CORE_TRACE("SDL window destroyed successfully.");
	}

	void Renderer::Init()
	{
		InitializeSDL();

		s_window = SDL_CreateWindow("Pixelate", 150, 150, 800, 600, SDL_WINDOW_SHOWN);

		if (s_window)
			PXL8_CORE_TRACE("SDL window created successfully.");
		else
			LogSDLError("SDL window creation failed: ");

		VulkanContext s_vulkan;
		CreateVulkanInstance(s_vulkan.Instance);

		CreateDebugMessenger(s_vulkan.Instance, &s_vulkan.DebugMessenger);

		SDL_Delay(3000);
	}

	void Renderer::Dispose()
	{
		s_vulkan.Dispose();
		DestroyWindow(s_window);
		SDL_Quit();

		PXL8_CORE_TRACE("Renderer disposed successfully.");
	}

	void VulkanContext::Dispose()
	{
		if (s_isDisposed)
			return;

		DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, 0);
		vkDestroyInstance(Instance, 0);
		s_isDisposed = true;
		PXL8_CORE_INFO("VkInstance destroyed successfully.");
	}

	VulkanContext::~VulkanContext()
	{
		Dispose();
	}
}
