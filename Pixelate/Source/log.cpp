#include "log.h"

namespace Pixelate {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_VulkanValidationLayerLogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("PIXL8_CORE");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stderr_color_mt("PIXL8_APP");
		s_ClientLogger->set_level(spdlog::level::trace);

		s_VulkanValidationLayerLogger = spdlog::stderr_color_mt("VULKAN_VALIDATION_LAYER");
		s_VulkanValidationLayerLogger->set_level(spdlog::level::trace);
	}
}