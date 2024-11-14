#include "render_graph.h"
#include "log.h"
#include "command_buffer_manager.h"
#include "queue_manager.h"

namespace Pixelate
{
	static PixelateRenderingInfo GetRenderingInfo(
		const PixelatePass& pass,
		const size_t index,
		const PixelateSwapchain& swapchain)
	{
		PixelateRenderingInfo renderingInfo{};

		if (!(pass.Flags & PIXELATE_PASS_COLOR_OUTPUT_TO_SWAPCHAIN))
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
						.imageView = VK_NULL_HANDLE, // swapchain image views are retrieved at runtime
						.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
						.clearValue = { .color = { 1.0f, 0.0f, 1.0f, 1.0f } },
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

		runtimePass.PassType = pass.PassType;
		runtimePass.Device = device.VkDevice;
		runtimePass.PassName = pass.Name;
		runtimePass.Flags = pass.Flags;

		for (int i = 0; i < PixelateSettings::MAX_FRAMES_IN_FLIGHT; i++)
		{
			runtimePass.CommandBuffer[i] = CommandBufferManager::GetCommandBuffer(
				device,
				CommandBufferDescriptor
				{
					.Type = CommandBufferType::GraphicsQueue,
					.Level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.PerformanceProfile = pass.Flags & PIXELATE_PASS_RECORD_ONCE
						? CommandBufferPerformanceProfile::PersistentResources
						: CommandBufferPerformanceProfile::Default,
				});

			runtimePass.RenderingInfos[i] = GetRenderingInfo(pass, i, swapchain);
		}

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

	static void RecordGraphicsPass(
		uint32_t frameInFlightIndex,
		uint32_t swapchainImageIndex,
		const PixelateSwapchain& swapchain,
		PixelateRuntimePass& runtimePass)
	{
		auto commandBuffer = runtimePass.CommandBuffer[frameInFlightIndex];

		//VkCommandBufferInheritanceRenderingInfo commandBufferInheritanceRenderingInfo
		//{
		//	.sType = ,
		//	.pNext = ,
		//	.flags = ,
		//	.viewMask = ,
		//	.colorAttachmentCount = ,
		//	.pColorAttachmentFormats = ,
		//	.depthAttachmentFormat = ,
		//	.stencilAttachmentFormat = ,
		//	.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		//};

		//VkCommandBufferInheritanceInfo commandBufferInheritanceInfo
		//{
		//	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		//	.pNext = &commandBufferInheritanceRenderingInfo,
		//	.renderPass = VK_NULL_HANDLE,
		//	.subpass = 0,
		//	.framebuffer = VK_NULL_HANDLE,
		//	.occlusionQueryEnable = VK_FALSE,
		//	.queryFlags = 0,
		//	.pipelineStatistics = 0,
		//};

		VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr, //&commandBufferInheritanceInfo,
		};
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		auto colorAttachmentCount = runtimePass.RenderingInfos[frameInFlightIndex].ColorAttachments.size();

		if (runtimePass.Flags & PIXELATE_PASS_COLOR_OUTPUT_TO_SWAPCHAIN && colorAttachmentCount == 1)
		{
			runtimePass.RenderingInfos[frameInFlightIndex].ColorAttachments[0].imageView = swapchain.SwapchainImageViews[swapchainImageIndex];
			runtimePass.RenderingInfos[frameInFlightIndex].RenderingInfo.pColorAttachments = runtimePass.RenderingInfos[frameInFlightIndex].ColorAttachments.data();
		}

		vkCmdBeginRendering(commandBuffer, &runtimePass.RenderingInfos[frameInFlightIndex].RenderingInfo);

		runtimePass.CommandBufferGraphics(commandBuffer, runtimePass.Pipeline);

		vkCmdEndRendering(commandBuffer);
		vkEndCommandBuffer(commandBuffer);
	}

	// TODO: add return values:
	// fences in order
	// semaphores in order
	// last swapchain image layout
	std::tuple<FenceGroup, PixelateSemaphore> RenderGraph::RecordAndSubmit(
		PixelateDevice device,
		uint32_t frameInFlightIndex,
		uint32_t swapchainImageIndex,
		VkSemaphoreSubmitInfo* pWaitSemaphores,
		uint32_t waitSemaphoreCount,
		const PixelateSwapchain& swapchain)
	{
		auto queueSubmitFences = FenceManager::GetFenceGroup(
			device.VkDevice,
			FenceGroupDescriptor
			{
				.Identifier = FenceIdenfitier::RenderGraphQueueSubmit,
				.FenceGroupSize = static_cast<uint32_t>(RuntimePasses.size())
			});

		auto swapchainImageReadyToPresentSemaphore = SemaphoreManager::GetSemaphore(
			device.VkDevice,
			VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			SemaphoreDescriptor{
				SemaphoreIdentifier::SwapchainImageReadyToPresent,
				frameInFlightIndex,
			});

		// Find the last operation in the render graph that outputs a swapchain image
		int lastSwapchainOperationIndex = 0;
		for (int i = 0; i < RuntimePasses.size(); i++)
			if (RuntimePasses[i].Flags & PIXELATE_PASS_COLOR_OUTPUT_TO_SWAPCHAIN)
				lastSwapchainOperationIndex = i;

		for (int i = 0; i < RuntimePasses.size(); i++)
		{
			auto& runtimePass = RuntimePasses[i];

			auto signalSemaphore = lastSwapchainOperationIndex == i ? &swapchainImageReadyToPresentSemaphore.SemaphoreSubmitInfo : nullptr;
			uint32_t signalSemaphoreCount = lastSwapchainOperationIndex == i;

			switch (runtimePass.PassType)
			{
			case PassType::Graphics:
				RecordGraphicsPass(frameInFlightIndex, swapchainImageIndex, swapchain, runtimePass); // TODO: Check if the pass is flagged with "record-once", and statetrack that shit

				QueueManager::GraphicsQueueSubmit(
					device,
					GraphicsQueueSubmitDescriptor(),
					runtimePass.CommandBuffer[frameInFlightIndex],
					queueSubmitFences[i],
					signalSemaphore, signalSemaphoreCount,
					pWaitSemaphores, waitSemaphoreCount);
				
				break;
				//TODO: implement other pass types
			}
		}

		return std::make_tuple(queueSubmitFences, swapchainImageReadyToPresentSemaphore);
	}
}

