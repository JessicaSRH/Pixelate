#pragma once

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Pixelate {
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetValidationLayerLogger() { return s_VulkanValidationLayerLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_VulkanValidationLayerLogger;
	};
}

#define LOG_VULKAN_VALIDATION_WARN(...)  ::Pixelate::Log::GetValidationLayerLogger()->warn(__VA_ARGS__)
#define LOG_VULKAN_VALIDATION_ERROR(...) ::Pixelate::Log::GetValidationLayerLogger()->error(__VA_ARGS__)
#define LOG_VULKAN_VALIDATION_INFO(...)  ::Pixelate::Log::GetValidationLayerLogger()->info(__VA_ARGS__)
#define PXL8_CORE_WARN(...)  ::Pixelate::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PXL8_CORE_ERROR(...) ::Pixelate::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PXL8_CORE_INFO(...)  ::Pixelate::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PXL8_WARN(...)  ::Pixelate::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PXL8_ERROR(...) ::Pixelate::Log::GetClientLogger()->error(__VA_ARGS__)
#define PXL8_INFO(...)  ::Pixelate::Log::GetClientLogger()->info(__VA_ARGS__)

#ifdef VERBOSE
#define LOG_VULKAN_VALIDATION_TRACE(...) ::Pixelate::Log::GetValidationLayerLogger()->trace(__VA_ARGS__)
#define PXL8_CORE_TRACE(...) ::Pixelate::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PXL8_TRACE(...) ::Pixelate::Log::GetClientLogger()->trace(__VA_ARGS__)
#else
#define LOG_VULKAN_VALIDATION_TRACE(...)
#define PXL8_CORE_TRACE(...)
#define PXL8_TRACE(...)
#endif
