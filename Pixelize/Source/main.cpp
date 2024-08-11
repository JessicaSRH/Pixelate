#include "pixelate_include.h"

using namespace Pixelate;

namespace Pixelize
{
	constexpr int WINDOW_OFFSET_X = SDL_WINDOWPOS_CENTERED;
	constexpr int WINDOW_OFFSET_Y = SDL_WINDOWPOS_CENTERED;
	constexpr int WINDOW_WIDTH = 1080;
	constexpr int WINDOW_HEIGHT = 720;
}

int main(int argc, char** argv)
{
	InitializePixelateLog();
	
	auto renderer = Renderer("Pixelize", Pixelize::WINDOW_OFFSET_X, Pixelize::WINDOW_OFFSET_Y, Pixelize::WINDOW_WIDTH, Pixelize::WINDOW_HEIGHT);
	
	std::cin.get();

	return 0;
}
