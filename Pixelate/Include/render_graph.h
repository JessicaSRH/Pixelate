#pragma once

#include "presentation_engine.h"
#include "pixelate_render_pass.h"
#include "pipeline_manager.h"
#include "pixelate_settings.h"
#include "semaphore_manager.h"
#include "fence_manager.h"

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
		const char* PassName;
		PassType PassType;
		PixelatePassFlags Flags;
		VkDevice Device;
		VkPipeline Pipeline;
		union
		{
			CommandGraphics CommandBufferGraphics;
			CommandHost CommandBufferHost;
		};
		VkCommandBuffer CommandBuffer[PixelateSettings::MAX_FRAMES_IN_FLIGHT];
		PixelateRenderingInfo RenderingInfos[PixelateSettings::MAX_FRAMES_IN_FLIGHT];
	};

	class RenderGraph
	{
	public:
		RenderGraph(PixelateDevice device, const RenderGraphDescriptor& descriptor, const PixelateSwapchain& swapchain);
		std::tuple<FenceGroup, PixelateSemaphore> RecordAndSubmit(
			PixelateDevice device,
			uint32_t frameInFlightIndex,
			uint32_t swapchainImageIndex,
			PixelateSemaphore acquireSwapchainImageSemaphore,
			const PixelateSwapchain& swapchain);
	private:
		std::vector<PixelateRuntimePass> RuntimePasses;

	};
}