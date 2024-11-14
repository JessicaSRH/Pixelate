#include "renderer.h"
#include "internal_pixelate_include.h"
#include "log.h"
#include "hasher.h"
#include "window.h"
#include "pixelate_device.h"
#include "semaphore_manager.h"
#include "fence_manager.h"
//#include "pixelate_helpers.h"
//#include "thread_safe_fifo_queue.h"
//#include "command_buffer_manager.h"
//#include "queue_manager.h"
//#include "resource_manager.h"
//#include "presentation_engine.h"
//#include "pipeline_manager.h"
//#include "pixelate_render_pass.h"
//#include "render_graph.h"

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

	static VpProfileProperties GetProfileProperties(const char* profileName = PixelateVulkanProfile::PROFILE_NAME, const int profileSpecVersion = PixelateVulkanProfile::PROFILE_SPEC_VERSION)
	{
		VpProfileProperties profile{};

		profile.specVersion = profileSpecVersion;
		strncpy_s(profile.profileName, profileName, strlen(profileName));

		return profile;
	}

	static PixelateInstance VulkanProfileBootstrap(const char* applicationName, const char* profileName = PixelateVulkanProfile::PROFILE_NAME, const int profileSpecVersion = PixelateVulkanProfile::PROFILE_SPEC_VERSION, unsigned int minApiVersion = PixelateVulkanProfile::PROFILE_MIN_API_VERSION)
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

	void Renderer::Render(RenderGraph renderGraph, std::function<bool()> inputHandler)
	{
		auto quit = false;
		while (!quit)
		{
			quit = inputHandler();

			auto frameInFlightFences = FenceManager::GetFenceGroup(
				m_Device.VkDevice,
				FenceGroupDescriptor
				{
					.Identifier = FenceIdenfitier::FrameHasBeenPresented,
					.FenceGroupSize = PixelateSettings::MAX_FRAMES_IN_FLIGHT,
					.CreateFlags = VK_FENCE_CREATE_SIGNALED_BIT
				});

			frameInFlightFences.Wait(m_FrameInFlightIndex);

			auto acquireSwapchainImageSemaphore = SemaphoreManager::GetSemaphore(
				m_Device.VkDevice,
				VK_PIPELINE_STAGE_2_NONE,
				SemaphoreDescriptor{
					SemaphoreIdentifier::SwapchainImageHasBeenAcquired,
					m_FrameInFlightIndex,
				});

			// -------------------------------------------------------
			// TODO: FIGURE OUT WHY swapchainImageIndex is always 0!!!
			auto swapchainImageIndex = m_Presentation.AcquireSwapcahinImage(acquireSwapchainImageSemaphore);
			//--------------------------------------------------------

			auto [fence, swapchainImageReadyToPresentSemaphore] = renderGraph.RecordAndSubmit(
				m_Device,
				m_FrameInFlightIndex,
				swapchainImageIndex,
				&acquireSwapchainImageSemaphore.SemaphoreSubmitInfo, 1,
				m_Presentation.GetSwapchain());

			m_Presentation.Present(
				swapchainImageIndex,
				&swapchainImageReadyToPresentSemaphore.SemaphoreSubmitInfo, 1
			);

			m_FrameInFlightIndex = (m_FrameInFlightIndex + 1) % PixelateSettings::MAX_FRAMES_IN_FLIGHT;
		}
	}

	RenderGraph Renderer::BuildRenderGraph(RenderGraphDescriptor& renderGraphDescriptor)
	{
		return RenderGraph(m_Device, renderGraphDescriptor, m_Presentation.GetSwapchain());
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