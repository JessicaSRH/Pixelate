#include "pixelate_window.h"

namespace Pixelate
{
	void LogSDLError(std::string msg)
	{
		auto errorString = msg + SDL_GetError();
		PXL8_APP_ERROR(errorString);
	}

	int InitializeSDL()
	{
		int result = SDL_Init(SDL_INIT_EVERYTHING);

		if (result)
		{
			LogSDLError("SDL2 initialization failed: ");
			return result;
		}

		PXL8_APP_TRACE("SDL initialized successfully.");

		return 0;
	}

	SDL_Window* InitializeSDLWindow(const char* name, int x, int y, int width, int height)
	{
		InitializeSDL();
		auto window = SDL_CreateWindow(name, x, y, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

		if (window)
			PXL8_APP_TRACE("SDL window created successfully.");
		else
			LogSDLError("SDL window creation failed: ");

		return window;
	}

	void DestroySDLWindow(SDL_Window*& window)
	{
		if (!window) // nothing to dispose
			return;

		SDL_DestroyWindow(window);
		SDL_Quit();
		window = 0;

		PXL8_APP_TRACE("SDL window destroyed successfully.");
	}
}