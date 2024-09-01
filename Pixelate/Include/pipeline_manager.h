#pragma once

#include "pixelate_render_pass.h"
#include "vma_usage.h"

namespace Pixelate
{
	namespace Pipelines
	{
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts(VkDevice device, GraphicsPipelineDescriptor& descriptor);
		VkPipelineLayout GetPipelineLayout(VkDevice device, GraphicsPipelineDescriptor& descriptor);
		VkPipeline GetGraphicsPipeline(
			VkDevice device,
			const PixelatePass& pass,
			VkViewport viewport,
			VkRect2D scissor,
			VkFormat swapchainFormat);
	}
}