#include "pipeline_manager.h"
#include "log.h"
#include "hasher.h"
#include "pixelate_helpers.h"

namespace Pixelate
{
	namespace Pipelines
	{
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts(VkDevice device, const GraphicsPipelineDescriptor& descriptor)
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

		VkPipelineLayout GetPipelineLayout(VkDevice device, const GraphicsPipelineDescriptor& descriptor)
		{
			auto descriptorSetLayouts = GetDescriptorSetLayouts(device, descriptor);

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.setLayoutCount = (uint32_t)descriptorSetLayouts.size(),
				.pSetLayouts = descriptorSetLayouts.data(),
				.pushConstantRangeCount = (uint32_t)descriptor.PushConstantRanges.size(),
				.pPushConstantRanges = descriptor.PushConstantRanges.data(),
			};

			VkPipelineLayout pipelineLayout{};
			auto result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to create pipeline layout!");

			return pipelineLayout;
		}

		static std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageInfo(VkDevice device, const GraphicsPipelineDescriptor& descriptor)
		{
			constexpr std::array<VkShaderStageFlagBits, 2> supportedGraphicsShaderStages
			{
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
			};

			std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

			for (const auto& shaderStage : supportedGraphicsShaderStages)
			{
				if (!(descriptor.ShaderDescriptor.ShaderStages & shaderStage))
				{
					PXL8_CORE_WARN("Shader stage not supported: " + shaderStage);
					continue;
				}

				std::string shaderPath = descriptor.ShaderDescriptor.Path;

				if (shaderPath.back() != '/')
					shaderPath += '/';

				shaderPath += descriptor.ShaderDescriptor.Name;
				switch (shaderStage)
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
					.stage = shaderStage,
					.module = shaderModule,
					.pName = "main",
				});
			}

			return shaderStages;
		}

		static std::tuple<VkPipelineColorBlendStateCreateInfo, std::vector<VkPipelineColorBlendAttachmentState>> GetColorBlendState(
			const GraphicsPipelineDescriptor& pipelineDescriptor,
			const std::vector<PixelateResourceUsage>& outputs)
		{
			std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates{};

			for (int i = 0; i < outputs.size(); i++)
				if (outputs[i].UsageFlags & PixelateResourceUsageFlags::PIXELATE_USAGE_COLOR_ATTACMENT)
					attachmentBlendStates.push_back(outputs[i].BlendState);

			// Store this in the pipeline state, but override some parts
			auto colorBlending = pipelineDescriptor.ColorBlendingState;
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.attachmentCount = attachmentBlendStates.size();
			colorBlending.pAttachments = attachmentBlendStates.data();

			return std::make_tuple(colorBlending, attachmentBlendStates);
		}

		struct PixelatePipelineRenderingCreateInfo
		{
			VkPipelineRenderingCreateInfo PipelineRenderingCreateInfo;
			std::vector<VkFormat> ColorAttachmentFormats{};
		};

		static PixelatePipelineRenderingCreateInfo GetPipelineRenderingInfo(const PixelatePass& pass, VkFormat swapchainFormat)
		{

			PixelatePipelineRenderingCreateInfo createInfo{};
			createInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;

			VkFormat format;
			for (int i = 0; i < pass.Outputs.size(); i++)
			{
				if (pass.Outputs[i].Resource.Type == PixelateResourceType::Buffer)
					continue; // not an attachment

				if ((pass.Flags & PIXELATE_PASS_COLOR_OUTPUT_TO_SWAPCHAIN)
					&& (pass.Outputs[i].UsageFlags & PIXELATE_USAGE_COLOR_ATTACMENT))
					format = swapchainFormat;
				else
					format = pass.Outputs[i].Resource.PhysicalImageDescriptor.Format;

				switch (pass.Outputs[i].UsageFlags)
				{
				case PixelateResourceUsageFlags::PIXELATE_USAGE_COLOR_ATTACMENT:
					createInfo.ColorAttachmentFormats.push_back(format);
					break;
				case PixelateResourceUsageFlags::PIXELATE_USAGE_DEPTH_ATTACMENT:
					createInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = format;
					break;
				}
			}

			createInfo.PipelineRenderingCreateInfo.colorAttachmentCount = createInfo.ColorAttachmentFormats.size();
			createInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = createInfo.ColorAttachmentFormats.data();

			return createInfo;
		}

		static VkPipeline CreateGraphicsPipeline(
			VkDevice device,
			const PixelatePass& pass,
			VkViewport& viewport,
			VkRect2D& scissor,
			VkFormat swapchainFormat)
		{
			auto shaderStages = GetPipelineShaderStageInfo(device, pass.GraphicsPipelineDescriptor);
			auto pipelineLayout = GetPipelineLayout(device, pass.GraphicsPipelineDescriptor);
			
			VkPipelineVertexInputStateCreateInfo vertexInputInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount = (uint32_t)pass.GraphicsPipelineDescriptor.VertexInputBindings.size(),
				.pVertexBindingDescriptions = pass.GraphicsPipelineDescriptor.VertexInputBindings.data(),
				.vertexAttributeDescriptionCount = (uint32_t)pass.GraphicsPipelineDescriptor.VertexInputAttributes.size(),
				.pVertexAttributeDescriptions = pass.GraphicsPipelineDescriptor.VertexInputAttributes.data(),
			};
			
			VkPipelineViewportStateCreateInfo viewportState =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.viewportCount = 1,
				.pViewports = &viewport,
				.scissorCount = 1,
				.pScissors = &scissor,
			};
			
			auto [colorBlendState, colorBlendAttachmentState] = GetColorBlendState(pass.GraphicsPipelineDescriptor, pass.Outputs);

			auto pipelineRenderingCreateInfo = GetPipelineRenderingInfo(pass, swapchainFormat);

			VkGraphicsPipelineCreateInfo pipelineInfo =
			{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = &pipelineRenderingCreateInfo.PipelineRenderingCreateInfo,
				.stageCount = (uint32_t)shaderStages.size(),
				.pStages = shaderStages.data(),
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &pass.GraphicsPipelineDescriptor.InputAssemby,
				.pViewportState = &viewportState,
				.pRasterizationState = &pass.GraphicsPipelineDescriptor.RasterizationState,
				.pMultisampleState = &pass.GraphicsPipelineDescriptor.MultisamplingState,
				.pDepthStencilState = &pass.GraphicsPipelineDescriptor.DepthStencilState,
				.pColorBlendState = &colorBlendState,
				.layout = pipelineLayout,
			};
			
			VkPipeline pipeline;
			auto result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
			
			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR(std::string("Failed to create pipeline with shader: ") + pass.GraphicsPipelineDescriptor.ShaderDescriptor.Name);

			PXL8_CORE_INFO(std::string("Pipeline created successfully for shader: ") + pass.GraphicsPipelineDescriptor.ShaderDescriptor.Name);

			return pipeline;
		}

		std::unordered_map<uint64_t, VkPipeline> g_Pipelines{};

		VkPipeline GetGraphicsPipeline(
			VkDevice device,
			const PixelatePass& pass,
			VkViewport viewport,
			VkRect2D scissor,
			VkFormat swapchainFormat)
		{
			auto hash = pass.GraphicsPipelineDescriptor.Hash();

			auto pipelineSearch = g_Pipelines.find(hash);
			if (pipelineSearch != g_Pipelines.end())
				return pipelineSearch->second;

			g_Pipelines.emplace(hash, CreateGraphicsPipeline(device, pass, viewport, scissor, swapchainFormat));
			
			return g_Pipelines.at(hash);
		}
	}
}