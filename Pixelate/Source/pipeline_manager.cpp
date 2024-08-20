#include "pipeline_manager.h"

namespace Pixelate
{
	namespace Pipelines
	{
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts(VkDevice device, PipelineDescriptor& descriptor)
		{
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
			descriptorSetLayouts.reserve(descriptor.DescriptorSetLayoutBindings.size());

			for (auto& descriptorBinding : descriptor.DescriptorSetLayoutBindings)
			{
				VkDescriptorSetLayoutCreateInfo layoutInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.bindingCount = static_cast<uint32_t>(descriptorBinding.size()),
					.pBindings = descriptorBinding.data(),
				};

				descriptorSetLayouts.emplace_back();
				if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts.back()) != VK_SUCCESS)
					PXL8_CORE_ERROR("Failed to create descriptor set layout!");
			}

			return descriptorSetLayouts;
		}

		VkPipelineLayout GetPipelineLayout(VkDevice device, PipelineDescriptor& descriptor)
		{
			auto descriptorSetLayouts = GetDescriptorSetLayouts(device, descriptor);

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.setLayoutCount = descriptorSetLayouts.size(),
				.pSetLayouts = descriptorSetLayouts.data(),
				.pushConstantRangeCount = descriptor.PushConstantRanges.size(),
				.pPushConstantRanges = descriptor.PushConstantRanges.data(),
			};

			VkPipelineLayout pipelineLayout{};
			auto result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to create pipeline layout!");

			return pipelineLayout;
		}

		std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageInfo(VkDevice device, PipelineDescriptor& descriptor)
		{
			constexpr std::array<VkShaderStageFlagBits, 2> supportedGraphicsShaderStages
			{
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
			};

			std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

			for (const auto shaderStageFlag : supportedGraphicsShaderStages)
			{
				if (!(descriptor.ShaderStageFlags & shaderStageFlag))
					return; // shader stage is not included

				std::string shaderPath = "spirv_";
				shaderPath += +descriptor.Shader;
				switch (shaderStageFlag)
				{
				case VK_SHADER_STAGE_VERTEX_BIT:
					shaderPath += "_vertex.spv";
					break;
				case VK_SHADER_STAGE_FRAGMENT_BIT:
					shaderPath += "_fragment.spv";
					break;
				}

				std::vector<char> spirvByteCode = Helpers::ReadFile(shaderPath);

				VkShaderModuleCreateInfo createInfo
				{
					.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
					.codeSize = spirvByteCode.size(),
					.pCode = reinterpret_cast<const uint32_t*>(spirvByteCode.data()),
				};

				VkShaderModule shaderModule;
				auto result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

				if (result != VK_SUCCESS)
					PXL8_CORE_ERROR("Failed to create shader module from spirv byte data!");

				shaderStages.emplace_back(VkPipelineShaderStageCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.stage = shaderStageFlag,
					.module = shaderModule,
					.pName = "main",
				});
			}

			return shaderStages;
		}

		VkPipeline CreateGraphicsPipeline(VkDevice device, PipelineDescriptor& descriptor)
		{
			auto shaderStages = GetPipelineShaderStageInfo(device, descriptor);
			auto pipelineLayout = GetPipelineLayout(device, descriptor);

			VkPipeline pipeline;
			vkCreateGraphicsPipelines();

		}

		VkPipeline GetGraphicsPipeline(VkDevice device, PipelineDescriptor& descriptor)
		{
			// hash pipeline and check if pipeline is created already

			// create pipeline if not
			return CreateGraphicsPipeline(device, descriptor);
		}
	}
}