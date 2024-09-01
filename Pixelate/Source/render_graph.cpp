#include "render_graph.h"
#include "log.h"
#include "command_buffer_manager.h"

namespace Pixelate
{
	static PixelateRenderingInfo GetRenderingInfo(
		const PixelatePass& pass,
		const size_t index,
		const PixelateSwapchain& swapchain)
	{
		PixelateRenderingInfo renderingInfo{};

		if (!(pass.Flags & PIXELATE_PASS_COLOR_ATTACHMENT_IS_SWAPCHAIN))
		{
			PXL8_CORE_WARN("Non-swapchain render targets are not currently supported! Rendering to swapchain...");
		}

		for (int i = 0; i < pass.Outputs.size(); i++)
		{
			if (pass.Outputs[i].UsageFlags & PIXELATE_USAGE_COLOR_ATTACMENT) // only color attachment is supported for now...
			{
				renderingInfo.ColorAttachments.push_back(
					VkRenderingAttachmentInfo
					{
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
						.imageView = swapchain.SwapchainImageViews[index], // only support swapchain color attachments for now...
						.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
						.clearValue = { .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
					});
			}
		}

		renderingInfo.RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.RenderingInfo.renderArea = { 0, 0, swapchain.Extent.width, swapchain.Extent.height };
		renderingInfo.RenderingInfo.layerCount = 1;
		renderingInfo.RenderingInfo.colorAttachmentCount = renderingInfo.ColorAttachments.size();
		renderingInfo.RenderingInfo.pColorAttachments = renderingInfo.ColorAttachments.data();
		renderingInfo.RenderingInfo.pDepthAttachment = renderingInfo.DepthAttachment.has_value() ? &renderingInfo.DepthAttachment.value() : nullptr;
		renderingInfo.RenderingInfo.pStencilAttachment = renderingInfo.StencilAttachment.has_value() ? &renderingInfo.StencilAttachment.value() : nullptr;
		
		return renderingInfo;
	}

	static PixelateRuntimePass BuildGraphicsPass(PixelateDevice device, const PixelatePass& pass, const PixelateSwapchain& swapchain)
	{
		PixelateRuntimePass runtimePass{};

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.Extent.width;
		viewport.height = (float)swapchain.Extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain.Extent;

		runtimePass.Pipeline = Pipelines::GetGraphicsPipeline(device.VkDevice, pass, viewport, scissor, swapchain.SurfaceFormat.format);

		//int renderingInfosCount = pass.Flags & PIXELATE_PASS_COLOR_ATTACHMENT_IS_SWAPCHAIN
		//	? swapchain.SwapchainImages.size()
		//	: PixelateRuntimePass::RenderingInfosCount;
		int renderingInfosCount = swapchain.SwapchainImages.size(); // remove this line when the above lines are reintroduced

		for (int i = 0; i < renderingInfosCount; i++)
			runtimePass.RenderingInfos[i] = GetRenderingInfo(pass, i, swapchain);

		runtimePass.PassType = pass.PassType;
		runtimePass.Device = device.VkDevice;
		runtimePass.PassName = pass.Name;
		runtimePass.Flags = pass.Flags;
		runtimePass.CommandBuffer = CommandBufferManager::GetCommandBuffer(
			device,
			CommandBufferDescriptor
			{
				.Type = CommandBufferType::GraphicsQueue,
				.Level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.PerformanceProfile = pass.Flags & PIXELATE_PASS_RECORD_ONCE
					? CommandBufferPerformanceProfile::PersistentResources
					: CommandBufferPerformanceProfile::Default,
			});

		switch (pass.PassType)
		{
		case PassType::Graphics:
			runtimePass.CommandBufferGraphics = pass.CommandBufferGraphics;
			break;
		case PassType::Host:
			runtimePass.CommandBufferHost = pass.CommandBufferHost;
			break;
		}

		return runtimePass;
	}

	RenderGraph::RenderGraph(PixelateDevice device, const RenderGraphDescriptor& renderGraphDescriptor, const PixelateSwapchain& swapchain)
	{
		if (!RuntimePasses.empty())
			RuntimePasses.clear();

		RuntimePasses.resize(renderGraphDescriptor.Passes.size());

		auto& passes = renderGraphDescriptor.Passes;

		for (int i = 0; i < renderGraphDescriptor.Passes.size(); i++)
		{
			switch (passes[i].PassType)
			{
			case PassType::Graphics:
				RuntimePasses[i] = BuildGraphicsPass(device, passes[i], swapchain);
				break;
			}
		}
	}

	static void RecordGraphicsPass(uint32_t index, const PixelateRuntimePass& runtimePass)
	{
		vkCmdBeginRendering(runtimePass.CommandBuffer, &runtimePass.RenderingInfos[index].RenderingInfo);

		runtimePass.CommandBufferGraphics(runtimePass.CommandBuffer, runtimePass.Pipeline);

		vkCmdEndRendering(runtimePass.CommandBuffer);
	}

	// TODO: add return values:
	// fences in order
	// semaphores in order
	// last swapchain image layout
	void RenderGraph::Render(uint32_t frameInFlightIndex, uint32_t swapchainImageIndex) const
	{

		//for (const auto& runtimePass : RuntimePasses)
		//{
		//	uint32_t index = runtimePass.Flags & PIXELATE_PASS_COLOR_ATTACHMENT_IS_SWAPCHAIN
		//		? swapchainImageIndex
		//		: frameInFlightIndex;

		//	RecordGraphicsPass(index, runtimePass); // TODO: Check if the pass is record-once, and figure out how to statetrack that shit

		//}

	}
}

