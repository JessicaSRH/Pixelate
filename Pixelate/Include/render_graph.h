#pragma once

#include "pixelate_render_pass.h"
#include "pipeline_manager.h"

namespace Pixelate
{
	struct RenderGraphDescriptor
	{
		std::vector<PixelatePass> Passes;
	};

	struct PixelateRuntimePass
	{
		const char* PassName;
		uint16_t SubpassIndex;
		VkDevice Device;
		VkPipeline Pipeline;
		VkRenderPass RenderPass;
		VkCommandBuffer CommandBuffer;
		PassType PassType;
		union
		{
			CommandGraphics CommandBufferGraphics;
			CommandHost CommandBufferHost;
		};
		PixelateCommandBufferFlags Flags;
	};

	class RenderGraph
	{
	public:
		RenderGraph(VkDevice device, const RenderGraphDescriptor& descriptor);
		void Render() const;
	private:
		std::vector<PixelateRuntimePass> Passes;

	};
}