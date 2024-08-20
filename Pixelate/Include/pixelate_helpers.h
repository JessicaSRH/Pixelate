#include <vector>
#include <fstream>
#include <stdexcept>

namespace Pixelate::Helpers
{
	std::vector<char> ReadFile(const std::string& filename);
}