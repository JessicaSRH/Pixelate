#include "pixelate_include.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

using namespace Pixelate;

namespace Pixelize
{
	constexpr int WINDOW_OFFSET_X = SDL_WINDOWPOS_CENTERED;
	constexpr int WINDOW_OFFSET_Y = SDL_WINDOWPOS_CENTERED;
	constexpr int WINDOW_WIDTH = 1080;
	constexpr int WINDOW_HEIGHT = 720;

	bool HandleInput()
	{
		SDL_Event e;
		auto quit = false;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}

			// User presses a key
			else if (e.type == SDL_KEYDOWN)
			{
				// Select action based on key pressed
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_UP:
					PXL8_CORE_INFO("Up arrow key pressed.");
					break;
				case SDLK_DOWN:
					PXL8_CORE_INFO("Down arrow key pressed.");
					break;
				case SDLK_LEFT:
					PXL8_CORE_INFO("Left arrow key pressed.");
					break;
				case SDLK_RIGHT:
					PXL8_CORE_INFO("Right arrow key pressed.");
					break;
				default:
					PXL8_CORE_INFO("Some other key pressed.");
					break;
				}
			}
		}

		return quit;
	}
}

void TrianglePassCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipelineHandle)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHandle);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

static PixelatePass GetTrianglePass()
{
	PixelatePass trianglePass(
		"TrianglePass",
		std::move(
			GraphicsPipelineDescriptor
			{
				.ShaderDescriptor =
				{
					PixelateShaderDescriptor
					{
						.Name = "triangle",
						.Path = "Pixelize/Resources/Shaders/",
						.ShaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					},
				}
			}
			),
			PIXELATE_PASS_COLOR_OUTPUT_TO_SWAPCHAIN | PIXELATE_PASS_RECORD_ONCE,
			TrianglePassCommandBuffer,
		{
			// Inputs if any
		},
		{
			PixelateResourceUsage
			{
				.Resource = PixelateResource
				{
					.Name = "MainColorAttachment",
				},
				.UsageFlags = PIXELATE_USAGE_COLOR_ATTACMENT,
			},
		}
		);

	return trianglePass;
}

int main(int argc, char** argv)
{	
	auto renderer = Pixelate::Renderer("Pixelize", Pixelize::WINDOW_OFFSET_X, Pixelize::WINDOW_OFFSET_Y, Pixelize::WINDOW_WIDTH, Pixelize::WINDOW_HEIGHT);

	Pixelate::RenderGraphDescriptor renderGraphDescriptor
	{
		.Passes =
		{
			 GetTrianglePass(),
		}
	};
	
	auto renderGraph = renderer.BuildRenderGraph(renderGraphDescriptor);

	renderer.Render(renderGraph, Pixelize::HandleInput);

	return 0;
}
