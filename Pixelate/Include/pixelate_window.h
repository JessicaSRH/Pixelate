#pragma once
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "pixelate_include.h"

namespace Pixelate
{
	void LogSDLError(std::string msg = "SDL Error: ");
	int InitializeSDL();
	SDL_Window* InitializeSDLWindow(const char* name, int x, int y, int width, int height);
	void DestroySDLWindow(SDL_Window*& window);
}