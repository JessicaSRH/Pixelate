#include "pixelate_helpers.h"
#include <log.h>
#include <filesystem>
#include <iostream>

namespace Pixelate::Helpers
{
	std::vector<char> ReadFile(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			auto currentWorkingDirectory = std::filesystem::current_path();
			PXL8_CORE_ERROR(std::string("Failed to open file at: ") +  filepath);
			std::cout << "Current working directory: " << currentWorkingDirectory << std::endl;
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
}