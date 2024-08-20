#pragma once

#include "internal_pixelate_include.h"

namespace Pixelate
{
	struct PipelineDescriptor
	{
		const char* Shader;
		VkShaderStageFlags ShaderStageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> DescriptorSetLayoutBindings;
		std::vector<VkPushConstantRange> PushConstantRanges;
		
		
	};

	namespace Pipelines
	{
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts(VkDevice device, PipelineDescriptor& descriptor);
		VkPipelineLayout GetPipelineLayout(VkDevice device, PipelineDescriptor& descriptor);
		VkPipeline GetGraphicsPipeline(VkDevice device, PipelineDescriptor& descriptor);
	}
}