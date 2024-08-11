#include "pixelate_renderer.h"

namespace Pixelate
{
	static void ValidateVulkanInstance(VkResult& result)
	{
		switch (result)
		{
			case VK_SUCCESS:
				PXL8_CORE_TRACE("Vulkan instance created successfully.");
				return;
			default:
				PXL8_CORE_ERROR("Failed to create Vulkan instance from Vulkan Profiles Toolset!");
		}
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

	static QueueFamilyIndices GetQueueFamilyIndices(const VkPhysicalDevice& physicalDevice)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices result{};

		auto i = 0;
		for (const auto& queueFamilyProperties : queueFamilies)
		{
			if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				result.GraphicsQueueFamily = i;

			if (queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
				result.ComputeQueueFamily = i;

			// TODO: Check for present support!

			i++;
		}

		return result;
	}

	static std::vector<VkExtensionProperties> GetSupportedDeviceExtensions(const VkPhysicalDevice& physicalDevice)
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		return availableExtensions;
	}

	static bool SupportsRayTracing(std::vector<VkExtensionProperties> availableExtensions)
	{
		bool rayTracingSupported = false;
		bool accelerationStructureSupported = false;
		bool rayTracingPipelineSupported = false;
		bool deferredHostOperationsSupported = false;

		for (const auto& extension : availableExtensions)
		{
			if (strcmp(extension.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
			{
				accelerationStructureSupported = true;
			}
			if (strcmp(extension.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
			{
				rayTracingPipelineSupported = true;
			}
			if (strcmp(extension.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
			{
				deferredHostOperationsSupported = true;
			}
		}

		rayTracingSupported = accelerationStructureSupported && rayTracingPipelineSupported && deferredHostOperationsSupported;

		return rayTracingPipelineSupported;
	}

	static int ScorePhysicalDevice(const VkPhysicalDevice& physicalDevice)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		auto score = 0;

		auto queueFamilyIndices = GetQueueFamilyIndices(physicalDevice);
		auto availableExtensions = GetSupportedDeviceExtensions(physicalDevice);

		score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0;
		score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? 500 : 0;
		score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 400 : 0;
		score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? 300 : 0;
		score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER ? 200 : 0;
		score += queueFamilyIndices.GraphicsQueueFamily.has_value() ? 50 : 0;
		score += queueFamilyIndices.ComputeQueueFamily.has_value() ? 10 : 0;
		score += SupportsRayTracing(availableExtensions) ? 1000 : 0;

		return score;
	}

	static VkPhysicalDevice PickPhysicalDevice(const std::vector<VkPhysicalDevice>& physicalDevices)
	{
		std::multimap<int, VkPhysicalDevice> scores{};

		for (const auto& physicalDevice : physicalDevices)
			scores.insert({ ScorePhysicalDevice(physicalDevice), physicalDevice } );

		if (scores.size() > 0)
			return scores.rbegin()->second;

		PXL8_CORE_ERROR("Failed to find suitable GPU!");

		return VK_NULL_HANDLE;
	}

	static VkPhysicalDevice CreatePhysicalDevice(
		const VkInstance& instance,
		std::function<VkPhysicalDevice(std::vector<VkPhysicalDevice>)> pickDeviceFunction = PickPhysicalDevice)
	{
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

		if (physicalDevices.size() == 0)
			PXL8_CORE_ERROR("Failed to find any GPUs!");

		physicalDevice = pickDeviceFunction(physicalDevices);

		return physicalDevice;
	}

	static VkPhysicalDevice PickPhysicalDeviceFromProfile(std::vector<VkPhysicalDevice> physicalDevices, VpCapabilities capabilities, VkInstance instance, const VpProfileProperties& profile, VkBool32* supported)
	{
		std::multimap<int, VkPhysicalDevice> scores{};

		for (const auto& physicalDevice : physicalDevices)
			scores.insert({ ScorePhysicalDevice(physicalDevice), physicalDevice });

		for (const auto& [score, physicalDevice] : std::ranges::reverse_view(scores))
		{
			auto result = vpGetPhysicalDeviceProfileSupport(capabilities, instance, physicalDevice, &profile, supported);
			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to query device profile support!");
			
			if (*supported == VK_TRUE)
				return physicalDevice;
		}

		auto profileName = &profile.profileName[0];

		PXL8_CORE_ERROR(std::string("No suitable GPU found for Vulkan profile: ") + profileName);

		return VK_NULL_HANDLE;
	}

	static VpCapabilities GetVpCapabilities()
	{
		VpCapabilities capabilities = VK_NULL_HANDLE;

		VpCapabilitiesCreateInfo createInfo;
		createInfo.apiVersion = VK_API_VERSION_1_3;
		createInfo.flags = VP_PROFILE_CREATE_STATIC_BIT;
		createInfo.pVulkanFunctions = nullptr;

		auto result = vpCreateCapabilities(&createInfo, nullptr, &capabilities);

		if (result != VK_SUCCESS)
			PXL8_CORE_ERROR("Failed to set Vulkan Profiles Toolset capabilities!");

		return capabilities;
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
			[](const auto& kvp) -> std::optional<const char*>
			{
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

	static VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, const Pixelate::QueueFamilyIndices& queueFamilyIndices, VpCapabilities capabilities, const VpProfileProperties& profile)
	{
		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos{};

		VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
		VkDeviceQueueCreateInfo computeQueueCreateInfo{};
		auto graphicsQueuePriority = 1.0f;
		auto computeQueuePriority = 1.0f;

		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsQueueFamily.value();
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;
		deviceQueueCreateInfos.push_back(graphicsQueueCreateInfo);

		if (queueFamilyIndices.ComputeQueueFamily.has_value())
		{
			computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			computeQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.ComputeQueueFamily.value();
			computeQueueCreateInfo.queueCount = 1;
			computeQueueCreateInfo.pQueuePriorities = &computeQueuePriority;
			deviceQueueCreateInfos.push_back(computeQueueCreateInfo);
		}

		VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();

		VpDeviceCreateInfo vpCreateInfo{};
		vpCreateInfo.pCreateInfo = &deviceCreateInfo;
		vpCreateInfo.enabledFullProfileCount = 1;
		vpCreateInfo.pEnabledFullProfiles = &profile;

		VkDevice device = VK_NULL_HANDLE;
		auto result = vpCreateDevice(capabilities, physicalDevice, &vpCreateInfo, nullptr, &device);
		if (result != VK_SUCCESS)
			PXL8_CORE_ERROR("Failed to create logical device!");

		PXL8_CORE_TRACE("Successfully created logical device.");

		return device;
	}

	static VulkanContext VulkanProfileBootstrap(const char* applicationName, const char* profileName = VP_KHR_ROADMAP_2022_NAME, const int profileSpecVersion = VP_KHR_ROADMAP_2022_SPEC_VERSION, unsigned int minApiVersion = VP_KHR_ROADMAP_2022_MIN_API_VERSION)
	{
		auto capabilities = GetVpCapabilities();

		VkResult result;
		VkBool32 supported = VK_FALSE;
		VpProfileProperties profile{};

		profile.specVersion = profileSpecVersion;
		strncpy_s(profile.profileName, profileName, strlen(profileName));
		
		result = vpGetInstanceProfileSupport(capabilities, nullptr, &profile, &supported);
		if (result != VK_SUCCESS)
			PXL8_CORE_ERROR("Failed to assess Vulkan profile support!");
		else if (supported != VK_TRUE)
			PXL8_CORE_ERROR("Vulkan profile is not supported by the device!");

		auto applicationInfo = VkApplicationInfo();
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = applicationName;
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.pEngineName = "Pixelate";
		applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.apiVersion = minApiVersion;

		std::vector<const char*> layers{};
		std::vector<const char*> extensions{};

		if (VALIDATION_LAYERS_ENABLED)
		{
			layers.emplace_back("VK_LAYER_KHRONOS_validation");
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			CheckInstanceLayerSupport(layers);
		}

		switch (PLATFORM)
		{
		case Platform::Win64:
			extensions.emplace_back(VK_KHR_surface_extension);
			extensions.emplace_back(VK_KHR_win32_surface_extension);
			break;
		default:
			PXL8_CORE_ERROR("Platform not supported! (How did you build this?)");
		}

		auto vkInstanceCreateInfo = VkInstanceCreateInfo();
		vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vkInstanceCreateInfo.pApplicationInfo = &applicationInfo;
		vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		vkInstanceCreateInfo.ppEnabledLayerNames = layers.data();
		vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		vkInstanceCreateInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfoStruct;
		if (VALIDATION_LAYERS_ENABLED)
		{
			debugMessengerInfoStruct = CreateDebugMessengerCreateInfoStruct();
			vkInstanceCreateInfo.pNext = &debugMessengerInfoStruct;
		}

		VpInstanceCreateInfo vpCreateInfo{};
		vpCreateInfo.pCreateInfo = &vkInstanceCreateInfo;
		vpCreateInfo.enabledFullProfileCount = 1;
		vpCreateInfo.pEnabledFullProfiles = &profile;

		VkInstance instance = VK_NULL_HANDLE;
		result = vpCreateInstance(capabilities, &vpCreateInfo, nullptr, &instance);
		ValidateVulkanInstance(result);
		
		auto physicalDevice = CreatePhysicalDevice(
			instance,
			[capabilities, instance, profile, &supported](const std::vector<VkPhysicalDevice>& physicalDevices) -> VkPhysicalDevice
			{
				return PickPhysicalDeviceFromProfile(physicalDevices, capabilities, instance, profile, &supported);
			});

		VulkanContext context{};
		context.Instance = instance;
		context.Device.VkPhysicalDevice = physicalDevice;
		CreateDebugMessenger(instance, &context.DebugMessenger);
		context.Device.QueueFamilyIndices = GetQueueFamilyIndices(physicalDevice);
		context.Device.VkDevice = CreateLogicalDevice(physicalDevice, context.Device.QueueFamilyIndices, capabilities, profile);

		PXL8_CORE_INFO(std::string("Vulkan device created from profile successfully:"));
		PXL8_CORE_INFO(std::string("    ") + profileName);
		PXL8_CORE_INFO(std::string("    Profile Version: ") + std::to_string(profileSpecVersion));

		return context;
	}

	PixelateSurface CreateSurface(VkInstance instance, const char* applicationName, int x, int y, int width, int height)
	{
		PixelateSurface surface{ width, height, InitializeSDLWindow(applicationName, x, y, width, height) };

		auto success = SDL_Vulkan_CreateSurface(surface.Window, instance, &surface.VkSurfaceKHR);

		if (!success)
			LogSDLError("SDL2 surface creation failed: ");
		else
			PXL8_CORE_INFO(std::string("Surface \"") + applicationName + "\" created successfully.");

		return surface;
	}

	Renderer::Renderer(const char* applicationName, int x, int y, int width, int height, const char* vulkanProfileName, const int profileSpecVersion, unsigned int minApiVersion) :
		m_Vulkan(VulkanProfileBootstrap(applicationName, vulkanProfileName, profileSpecVersion, minApiVersion)),
		m_Surface(CreateSurface(m_Vulkan.Instance, applicationName, x, y, width, height))
	{
	}

	Renderer::~Renderer()
	{
		if (m_Vulkan.Instance == VK_NULL_HANDLE)
			return; // is disposed already

		vkDestroySurfaceKHR(m_Vulkan.Instance, m_Surface.VkSurfaceKHR, nullptr);
		vkDestroyDevice(m_Vulkan.Device.VkDevice, nullptr);
		DestroySDLWindow(m_Surface.Window);
		DestroyDebugUtilsMessengerEXT(m_Vulkan.Instance, m_Vulkan.DebugMessenger, 0);
		vkDestroyInstance(m_Vulkan.Instance, 0);

		PXL8_CORE_TRACE("Renderer disposed successfully.");
	}
}

namespace PixelateDeprecated
{
	static VkInstance CreateVulkanInstance()
	{
		VkInstance instance;

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

		Pixelate::CheckInstanceLayerSupport(layers);

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
			debugMessengerInfoStruct = Pixelate::CreateDebugMessengerCreateInfoStruct();
			createInfo.pNext = &debugMessengerInfoStruct;
		}

		auto result = vkCreateInstance(&createInfo, 0, &instance);

		Pixelate::ValidateVulkanInstance(result);

		return instance;
	}
}