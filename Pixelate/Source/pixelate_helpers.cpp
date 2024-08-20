#include "pixelate_helpers.h"
#include <log.h>

namespace Pixelate::Helpers
{
	std::vector<char> ReadFile(const std::string& filepath)
	{
		{
			std::ifstream file(filepath, std::ios::ate | std::ios::binary);

			if (!file.is_open())
				PXL8_CORE_ERROR("Failed to open file at: " + filepath);

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();
			return buffer;
		}
	}
}