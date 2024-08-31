#pragma once

#include "vma_usage.h"

namespace Pixelate
{
	void LogSDLError(std::string msg = "SDL Error: ");
	int InitializeSDL();
	SDL_Window* InitializeSDLWindow(const char* name, int x, int y, int width, int height);
	void DestroySDLWindow(SDL_Window*& window);
}