#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>


#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "vulkan/vulkan.h"

#include "log.h"
#include "renderer.h"

#ifdef DEBUG
#define VALIDATION_LAYERS_ENABLED true
#else
#define VALIDATION_LAYERS_ENABLED false
#endif