#pragma once

#include "presentation_engine.h"
#include "pixelate_render_pass.h"
#include "pipeline_manager.h"

namespace Pixelate
{

	struct RenderGraphDescriptor
	{
		std::vector<PixelatePass> Passes{};
	};

	struct PixelateRenderingInfo
	{
		VkRenderingInfo RenderingInfo{};
		std::vector<VkRenderingAttachmentInfo> ColorAttachments{};
		std::optional<VkRenderingAttachmentInfo> DepthAttachment = std::nullopt;;
		std::optional<VkRenderingAttachmentInfo> StencilAttachment = std::nullopt;
	};

	struct PixelateRuntimePass
	{
		static constexpr int RenderingInfosCount = 3;// allocate room for up to 3 different render targets

		const char* PassName;
		PassType PassType;
		PixelatePassFlags Flags;
		VkCommandBuffer CommandBuffer;
		VkDevice Device;
		VkPipeline Pipeline;
		union
		{
			CommandGraphics CommandBufferGraphics;
			CommandHost CommandBufferHost;
		};
		PixelateRenderingInfo RenderingInfos[RenderingInfosCount];
	};

	class RenderGraph
	{
	public:
		RenderGraph(PixelateDevice device, const RenderGraphDescriptor& descriptor, const PixelateSwapchain& swapchain);
		void Render(uint32_t frameInFlightIndex, uint32_t swapchainImageIndex) const;
	private:
		std::vector<PixelateRuntimePass> RuntimePasses;

	};
}