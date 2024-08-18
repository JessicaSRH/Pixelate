#include "renderer.h"

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

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
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

	static void DestroyDebugUtilsMessengerEXT(const VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static QueueFamilyIndices GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface = VK_NULL_HANDLE)
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

			if (surface != VK_NULL_HANDLE && !result.PresentQueueFamily.has_value())
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

				if (presentSupport == VK_TRUE)
					result.PresentQueueFamily = i;
			}

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
	
	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.Formats.data());
		}
		else
		{
			PXL8_CORE_ERROR("No suported surface formats found on physical device!");
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.PresentModes.data());
		}
		return details;
	}

	static bool SupportsSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkExtensionProperties> availableExtensions)
	{
		auto swapchainExtensionSupported = false;
		for (const auto& extension : availableExtensions)
			if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				swapchainExtensionSupported = true;

		bool swapchainAdequate = false;
		if (swapchainExtensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
			swapchainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

		return swapchainExtensionSupported && swapchainAdequate;
	}

	static int ScorePhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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
		score += queueFamilyIndices.GraphicsQueueFamily.has_value() ? 500 : 0;
		score += queueFamilyIndices.ComputeQueueFamily.has_value() ? 100 : 0;
		score += SupportsRayTracing(availableExtensions) ? 1000 : 0;
		score += SupportsSwapchain(physicalDevice, surface, availableExtensions) ? 1000 : 0;

		return score;
	}

	static VkPhysicalDevice PickPhysicalDevice(VkSurfaceKHR surface, const std::vector<VkPhysicalDevice>& physicalDevices)
	{
		std::multimap<int, VkPhysicalDevice> scores{};

		for (const auto& physicalDevice : physicalDevices)
			scores.insert({ ScorePhysicalDevice(physicalDevice, surface), physicalDevice } );

		if (scores.size() > 0)
			return scores.rbegin()->second;

		PXL8_CORE_ERROR("Failed to find suitable GPU!");

		return VK_NULL_HANDLE;
	}

	static VkPhysicalDevice CreatePhysicalDevice(
		VkInstance instance,
		VkSurfaceKHR surface,
		std::function<VkPhysicalDevice(VkSurfaceKHR, std::vector<VkPhysicalDevice>)> pickDeviceFunction = PickPhysicalDevice)
	{
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

		if (physicalDevices.size() == 0)
			PXL8_CORE_ERROR("Failed to find any GPUs!");

		physicalDevice = pickDeviceFunction(surface, physicalDevices);

		return physicalDevice;
	}

	static VkPhysicalDevice PickPhysicalDeviceFromProfile(PixelateInstance& context, VkSurfaceKHR surface, std::vector<VkPhysicalDevice> physicalDevices, VkBool32* supported)
	{
		std::multimap<int, VkPhysicalDevice> scores{};

		for (const auto& physicalDevice : physicalDevices)
			scores.insert({ ScorePhysicalDevice(physicalDevice, surface), physicalDevice });

		for (const auto& [score, physicalDevice] : std::ranges::reverse_view(scores))
		{
			auto result = vpGetPhysicalDeviceProfileSupport(context.ProfileCapabilities, context.Instance, physicalDevice, &context.ProfileProperties, supported);
			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to query device profile support!");
			
			if (*supported == VK_TRUE)
				return physicalDevice;
		}

		auto profileName = &context.ProfileProperties.profileName[0];

		PXL8_CORE_ERROR(std::string("No suitable GPU found for Vulkan profile: ") + profileName);

		return VK_NULL_HANDLE;
	}

	static VpCapabilities GetVpCapabilities()
	{
		VpCapabilities Capabilities = VK_NULL_HANDLE;

		VpCapabilitiesCreateInfo createInfo;
		createInfo.apiVersion = VK_API_VERSION_1_3;
		createInfo.flags = VP_PROFILE_CREATE_STATIC_BIT;
		createInfo.pVulkanFunctions = nullptr;

		auto result = vpCreateCapabilities(&createInfo, nullptr, &Capabilities);

		if (result != VK_SUCCESS)
			PXL8_CORE_ERROR("Failed to set Vulkan Profiles Toolset capabilities!");

		return Capabilities;
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

	static VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, const Pixelate::QueueFamilyIndices& queueFamilyIndices, VpCapabilities Capabilities, const VpProfileProperties& profile)
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

		std::vector<const char*> additionaDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(additionaDeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = additionaDeviceExtensions.data();

		VpDeviceCreateInfo vpCreateInfo{};
		vpCreateInfo.pCreateInfo = &deviceCreateInfo;
		vpCreateInfo.enabledFullProfileCount = 1;
		vpCreateInfo.pEnabledFullProfiles = &profile;

		VkDevice device = VK_NULL_HANDLE;
		auto result = vpCreateDevice(Capabilities, physicalDevice, &vpCreateInfo, nullptr, &device);
		if (result != VK_SUCCESS)
			PXL8_CORE_ERROR("Failed to create logical device!");

		PXL8_CORE_TRACE("Successfully created logical device.");

		return device;
	}

	static VpProfileProperties GetProfileProperties(const char* profileName = VP_KHR_ROADMAP_2022_NAME, const int profileSpecVersion = VP_KHR_ROADMAP_2022_SPEC_VERSION)
	{
		VpProfileProperties profile{};

		profile.specVersion = profileSpecVersion;
		strncpy_s(profile.profileName, profileName, strlen(profileName));

		return profile;
	}

	static PixelateInstance VulkanProfileBootstrap(const char* applicationName, const char* profileName = VP_KHR_ROADMAP_2022_NAME, const int profileSpecVersion = VP_KHR_ROADMAP_2022_SPEC_VERSION, unsigned int minApiVersion = VP_KHR_ROADMAP_2022_MIN_API_VERSION)
	{
		Log::Init();

		auto capabilities = GetVpCapabilities();

		VkResult result;
		VkBool32 supported = VK_FALSE;
		auto profile = GetProfileProperties();

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
		
		PixelateInstance context{ instance, profile, capabilities };
		CreateDebugMessenger(instance, &context.DebugMessenger);

		return context;
	}

	static PixelateDevice CreatePixelateDevice(PixelateInstance& context, VkSurfaceKHR surface)
	{
		VkBool32 supported = VK_FALSE;

		auto physicalDevice = CreatePhysicalDevice(
			context.Instance,
			surface,
			[&context, &supported](VkSurfaceKHR surface, const std::vector<VkPhysicalDevice>& physicalDevices) -> VkPhysicalDevice
			{
				return PickPhysicalDeviceFromProfile(context, surface, physicalDevices, &supported);
			});

		PixelateDevice device;
		device.VkPhysicalDevice = physicalDevice;
		device.QueueFamilyIndices = GetQueueFamilyIndices(physicalDevice, surface);
		device.VkDevice = CreateLogicalDevice(physicalDevice, device.QueueFamilyIndices, context.ProfileCapabilities, context.ProfileProperties);

		PXL8_CORE_INFO(std::string("Vulkan device created from profile successfully:"));
		PXL8_CORE_INFO(std::string("    ") + context.ProfileProperties.profileName);
		PXL8_CORE_INFO(std::string("    Profile Version: ") + std::to_string(context.ProfileProperties.specVersion));

		return device;
	}

	static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == PixelateSettings::PREFERRED_SWAPCHAIN_IMAGE_FORMAT
				&& availableFormat.colorSpace == PixelateSettings::PREFERRED_SWAPCHAIN_COLOR_SPACE)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	static VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR; // support guaranteed
	}

	static VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& Capabilities, SDL_Window* window)
	{
		if (Capabilities.currentExtent.width != UINT32_MAX)
		{
			return Capabilities.currentExtent;
		}
		else
		{
			int width, height;
			SDL_Vulkan_GetDrawableSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(Capabilities.minImageExtent.width, std::min(Capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(Capabilities.minImageExtent.height, std::min(Capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	static void ValidateSwapchainCreation(VkResult result)
	{
		switch (result)
		{
		case VK_SUCCESS:
			PXL8_CORE_TRACE("Swapchain created succesfully.");
			break;
		case VK_SUBOPTIMAL_KHR:
			PXL8_CORE_WARN("Swapchain is suboptimal.");
			break;
		default:
			PXL8_CORE_ERROR("Failed to create swapchain.");
		}
	}

	PixelateSwapchain::PixelateSwapchain(PixelateDevice device, VkSurfaceKHR surface, SDL_Window* window) :
		SupportDetails(QuerySwapChainSupport(device.VkPhysicalDevice, surface)),
		SurfaceFormat(ChooseSwapchainSurfaceFormat(SupportDetails.Formats)),
		PresentMode(ChooseSwapchainPresentMode(SupportDetails.PresentModes)),
		Extent(ChooseSwapchainExtent(SupportDetails.Capabilities, window)),
		VkSwapchain(VK_NULL_HANDLE),
		m_Device(device),
		m_Surface(surface)
	{
		Recreate();
	}

	void PixelateSwapchain::Dispose()
	{
		DisposeImageViews();
		vkDestroySwapchainKHR(m_Device.VkDevice, VkSwapchain, nullptr);
	}

	void PixelateSwapchain::DisposeImageViews()
	{
		for (auto imageView : SwapchainImageViews)
			vkDestroyImageView(m_Device.VkDevice, imageView, nullptr);

		SwapchainImageViews.clear();
	}

	void PixelateSwapchain::Recreate()
	{
		DisposeImageViews();

		uint32_t imageCount = std::clamp((uint32_t)3, (uint32_t)SupportDetails.Capabilities.minImageCount, (uint32_t)SupportDetails.Capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = m_Surface;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = SurfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent = Extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.preTransform = SupportDetails.Capabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = PresentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VkSwapchain;

		QueueFamilyIndices indices = GetQueueFamilyIndices(m_Device.VkPhysicalDevice, m_Surface);
		uint32_t queueFamilyIndices[] = { indices.GraphicsQueueFamily.value(), indices.PresentQueueFamily.value() };

		if (indices.GraphicsQueueFamily != indices.PresentQueueFamily)
		{
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
			swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		ValidateSwapchainCreation(vkCreateSwapchainKHR(m_Device.VkDevice, &swapchainCreateInfo, nullptr, &VkSwapchain));

		uint32_t actualImageCount{};
		vkGetSwapchainImagesKHR(m_Device.VkDevice, VkSwapchain, &actualImageCount, nullptr);

		SwapchainImages.resize(actualImageCount);
		vkGetSwapchainImagesKHR(m_Device.VkDevice, VkSwapchain, &actualImageCount, SwapchainImages.data());

		SwapchainImageViews.resize(SwapchainImages.size());
		for (auto i = 0; i < actualImageCount; i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = SurfaceFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.image = SwapchainImages[0];

			if (vkCreateImageView(m_Device.VkDevice, &createInfo, nullptr, &SwapchainImageViews[i]) != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to create swapchain image views!");
			else
				PXL8_CORE_TRACE(std::string("Swapchain image view ") + std::to_string(i) + " created successfully.");
		}
	}

	PixelatePresentationEngine CreateSurface(VkInstance instance, const char* applicationName, int x, int y, int width, int height)
	{
		VkSurfaceKHR surface;
		auto window = InitializeSDLWindow(applicationName, x, y, width, height);
		auto success = SDL_Vulkan_CreateSurface(window, instance, &surface);

		if (!success)
			LogSDLError("SDL2 surface creation failed: ");
		else
			PXL8_CORE_INFO(std::string("Surface \"") + applicationName + "\" created successfully.");

		PixelatePresentationEngine presentationEngine(width, height, window, surface);

		return presentationEngine;
	}

	Renderer::Renderer(const char* applicationName, int x, int y, int width, int height, const char* vulkanProfileName, const int profileSpecVersion, unsigned int minApiVersion) :
		m_Instance(VulkanProfileBootstrap(applicationName, vulkanProfileName, profileSpecVersion, minApiVersion)),
		m_Presentation(CreateSurface(m_Instance.Instance, applicationName, x, y, width, height)),
		m_Device(CreatePixelateDevice(m_Instance, m_Presentation.GetSurface())),
		m_VulkanResourceManager(VulkanResourceManager(m_Instance.Instance, m_Device.VkDevice, m_Device.VkPhysicalDevice, minApiVersion))
	{
		m_Presentation.Initialize(m_Device);
	}

	void Renderer::Render(std::function<bool()> inputHandler)
	{
		auto quit = false;
		while (!quit)
		{
			quit = inputHandler();

			auto presentationFences = FenceManager::GetFenceGroup(
				m_Device.VkDevice,
				FenceGroupDescriptor
				{
					.Identifier = FenceIdenfitier::FrameHasBeenPresented,
					.FenceGroupSize = PixelateSettings::MAX_FRAMES_IN_FLIGHT,
					.CreateFlags = VK_FENCE_CREATE_SIGNALED_BIT
				});

			presentationFences.Wait(m_FrameInFlightIndex);

			auto acquireSwapchainImageSemaphore = SemaphoreManager::GetSemaphore(
				m_Device.VkDevice,
				VK_PIPELINE_STAGE_2_NONE,
				SemaphoreDescriptor{
					SemaphoreIdentifier::SwapchainImageHasBeenAcquired,
					m_FrameInFlightIndex,
				});

			auto [swapchainImageIndex, swapchainImageView] = m_Presentation.AcquireSwapcahinImage(acquireSwapchainImageSemaphore);

			// record command buffers that render (or copy/blit) to swapchain image
			// submit command buffers to graphics queue
				// - assign frame-in-flight fence to be signaled on completion
				// - assign frame-in-flight semaphore to be signaled on completion

			m_Presentation.Present(swapchainImageIndex, PixelateSemaphore());

			m_FrameInFlightIndex = (m_FrameInFlightIndex + 1) % PixelateSettings::MAX_FRAMES_IN_FLIGHT;
		}
	}

	const SDL_Window* Renderer::GetWindow() const
	{
		return m_Presentation.GetWindow();
	}

	Renderer::~Renderer()
	{
		if (m_Instance.Instance == VK_NULL_HANDLE)
			return; // is disposed already

		FenceManager::Dispose();
		SemaphoreManager::Dispose(m_Device.VkDevice);
		m_Presentation.Dispose(m_Instance.Instance);
		vkDestroyDevice(m_Device.VkDevice, nullptr);
		DestroyDebugUtilsMessengerEXT(m_Instance.Instance, m_Instance.DebugMessenger, 0);
		vkDestroyInstance(m_Instance.Instance, 0);

		PXL8_CORE_TRACE("Renderer disposed successfully.");
	}

	// Presentation engine

	PixelatePresentationEngine::PixelatePresentationEngine(int width, int height, SDL_Window* window, VkSurfaceKHR surface)
		: m_Width(width), m_Height(height), m_Window(window), m_VkSurfaceKHR(surface), m_Swapchain()
	{
	}

	void PixelatePresentationEngine::Initialize(PixelateDevice device)
	{
		m_Device = device;

		vkGetDeviceQueue(m_Device.VkDevice, m_Device.QueueFamilyIndices.PresentQueueFamily.value(), 0, &m_PresentQueue);;

		m_Swapchain = PixelateSwapchain(device, m_VkSurfaceKHR, m_Window);
	}

	std::tuple<uint32_t, VkImageView> PixelatePresentationEngine::AcquireSwapcahinImage(VkSemaphore signalSemaphore, VkFence signalFence)
	{
		uint32_t imageIndex{};
		auto result = vkAcquireNextImageKHR(m_Device.VkDevice, m_Swapchain.VkSwapchain, std::numeric_limits<uint64_t>::max(), signalSemaphore, signalFence, &imageIndex);

		return { imageIndex, m_Swapchain.SwapchainImageViews[imageIndex]};
	}

	void ValidateSwapchainResult(VkResult result)
	{
		if (result == VK_SUBOPTIMAL_KHR)
			PXL8_APP_WARN("Swapchain is suboptimal!");

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			PXL8_APP_WARN("Swapchain is out of date!");

		if (result == VK_ERROR_DEVICE_LOST)
			PXL8_APP_WARN("Device was lost during swapchain presentation!");

		if (result != VK_SUCCESS)
			PXL8_APP_WARN("Unknown error during present queue submission!");
	}

	void PixelatePresentationEngine::TransitionImageLayout(
		uint32_t swapchainImageIndex,
		PixelateSemaphore signalSemaphore,
		VkImageLayout newLayout,
		VkImageLayout oldLayout)
	{


		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_Swapchain.SwapchainImages[swapchainImageIndex];
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
		barrier.dstAccessMask = 0;

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

		auto signalSemaphoreSubmitInfo = (VkSemaphoreSubmitInfo)signalSemaphore;

		auto fenceGroup = FenceManager::GetFenceGroup(
			m_Device.VkDevice,
			FenceGroupDescriptor
			{
				.Identifier = FenceIdenfitier::SwapchainLayoutTransition,
				.FenceGroupSize = static_cast<uint32_t>(m_Swapchain.SwapchainImages.size()),
			});

		auto commandBuffer = CommandBufferManager::GetCommandBuffer(m_Device);
		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
		fenceGroup.AddFencedCallback([commandBuffer]() mutable { commandBuffer.Return(); }, swapchainImageIndex);

		QueueManager::GraphicsQueueSubmit(
			m_Device,
			GraphicsQueueSubmitDescriptor{ GraphicsQueueType::Default },
			commandBuffer.CommandBuffer,
			fenceGroup[swapchainImageIndex],
			&signalSemaphoreSubmitInfo, 1,
			nullptr, 0
			);
	}

	void PixelatePresentationEngine::Present(
		uint32_t swapchainImageIndex,
		PixelateSemaphore waitSemaphore,
		VkImageLayout previousLayout)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain.VkSwapchain;
		presentInfo.pImageIndices = &swapchainImageIndex;

		std::vector<VkSemaphore> waitSemaphores{ waitSemaphore };

		if (!(previousLayout & (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR | VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)))
		{
			auto imageTransitionSemaphore = SemaphoreManager::GetSemaphore(
				m_Device.VkDevice,
				VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
				SemaphoreDescriptor
				{
					.Identifier = SemaphoreIdentifier::SwapchainImageTransitionToPresent,
					.Index = swapchainImageIndex
				});
			waitSemaphores.push_back(imageTransitionSemaphore);
			TransitionImageLayout(swapchainImageIndex, imageTransitionSemaphore);
		}

		presentInfo.waitSemaphoreCount = waitSemaphores.size();
		presentInfo.pWaitSemaphores = waitSemaphores.data();

		ValidateSwapchainResult(vkQueuePresentKHR(m_PresentQueue, &presentInfo));
	}

	void PixelatePresentationEngine::Dispose(VkInstance instance)
	{
		m_Swapchain.Dispose();
		vkDestroySurfaceKHR(instance, m_VkSurfaceKHR, nullptr);
		DestroySDLWindow(m_Window);
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