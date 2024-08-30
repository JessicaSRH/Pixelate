#include "pipeline_manager.h"
#include "log.h"
#include "hasher.h"
#include "pixelate_helpers.h"

namespace Pixelate
{

	GraphicsPipelineDescriptor::GraphicsPipelineDescriptor(const GraphicsPipelineDescriptor& other) :
		Shader(other.Shader),
		ShaderStageFlags(other.ShaderStageFlags),
		DescriptorSetLayoutBindings(other.DescriptorSetLayoutBindings),
		PushConstantRanges(other.PushConstantRanges),
		VertexInputBindings(other.VertexInputBindings),
		VertexInputAttributes(other.VertexInputAttributes),
		InputAssemby(other.InputAssemby),
		RasterizationState(other.RasterizationState),
		MultisamplingState(other.MultisamplingState),
		DepthStencilState(other.DepthStencilState),
		ColorBlendingState(other.ColorBlendingState)
	{

		if (!other.DescriptorSetLayoutBindings.empty())
			DescriptorSetLayoutBindings = other.DescriptorSetLayoutBindings;

		if (!other.PushConstantRanges.empty())
			PushConstantRanges = other.PushConstantRanges;

		if (!other.VertexInputBindings.empty())
			VertexInputBindings = other.VertexInputBindings;

		if (!other.VertexInputAttributes.empty())
			VertexInputAttributes = other.VertexInputAttributes;

	}

	GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::operator=(const GraphicsPipelineDescriptor& other) noexcept
	{
		if (this != &other)
		{
			Shader = other.Shader;
			ShaderStageFlags = other.ShaderStageFlags;

			if (!other.DescriptorSetLayoutBindings.empty())
				DescriptorSetLayoutBindings = other.DescriptorSetLayoutBindings;

			if (!other.PushConstantRanges.empty())
				PushConstantRanges = other.PushConstantRanges;

			if (!other.VertexInputBindings.empty())
				VertexInputBindings = other.VertexInputBindings;

			if (!other.VertexInputAttributes.empty())
				VertexInputAttributes = other.VertexInputAttributes;

			InputAssemby = other.InputAssemby;
			RasterizationState = other.RasterizationState;
			MultisamplingState = other.MultisamplingState;
			DepthStencilState = other.DepthStencilState;
			ColorBlendingState = other.ColorBlendingState;
		}
		return *this;
	}

	GraphicsPipelineDescriptor::GraphicsPipelineDescriptor(GraphicsPipelineDescriptor&& other) noexcept :
		Shader(other.Shader),
		ShaderStageFlags(other.ShaderStageFlags),
		DescriptorSetLayoutBindings(std::move(other.DescriptorSetLayoutBindings)),
		PushConstantRanges(std::move(other.PushConstantRanges)),
		VertexInputBindings(std::move(other.VertexInputBindings)),
		VertexInputAttributes(std::move(other.VertexInputAttributes)),
		InputAssemby(other.InputAssemby),
		RasterizationState(other.RasterizationState),
		MultisamplingState(other.MultisamplingState),
		DepthStencilState(other.DepthStencilState),
		ColorBlendingState(other.ColorBlendingState)
	{
	}

	GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::operator=(GraphicsPipelineDescriptor && other) noexcept
	{
		if (this != &other)
		{
			Shader = other.Shader;
			ShaderStageFlags = other.ShaderStageFlags;

			if (!other.DescriptorSetLayoutBindings.empty())
				DescriptorSetLayoutBindings = std::move(other.DescriptorSetLayoutBindings);

			if (!other.PushConstantRanges.empty())
				PushConstantRanges = std::move(other.PushConstantRanges);

			if (!other.VertexInputBindings.empty())
				VertexInputBindings = std::move(other.VertexInputBindings);

			if (!other.VertexInputAttributes.empty())
				VertexInputAttributes = std::move(other.VertexInputAttributes);

			InputAssemby = other.InputAssemby;
			RasterizationState = other.RasterizationState;
			MultisamplingState = other.MultisamplingState;
			DepthStencilState = other.DepthStencilState;
			ColorBlendingState = other.ColorBlendingState;
		}
		return *this;
	}

	uint64_t GraphicsPipelineDescriptor::Hash() const
	{
		Hasher hasher;

		hasher.Hash(Shader);
		hasher.Hash(ShaderStageFlags);

		hasher.Hash((const char*)DescriptorSetLayoutBindings.data(), sizeof(DescriptorSetLayoutBindings) * DescriptorSetLayoutBindings.size());
		hasher.Hash((const char*)PushConstantRanges.data(), sizeof(PushConstantRanges) * PushConstantRanges.size());
		hasher.Hash((const char*)VertexInputBindings.data(), sizeof(VkVertexInputBindingDescription) * VertexInputBindings.size());
		hasher.Hash((const char*)VertexInputAttributes.data(), sizeof(VkVertexInputAttributeDescription) * VertexInputAttributes.size());

		hasher.Hash((const char*) &InputAssemby, sizeof(VkPipelineInputAssemblyStateCreateInfo));
		hasher.Hash((const char*)&RasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo));
		hasher.Hash((const char*)&MultisamplingState, sizeof(VkPipelineMultisampleStateCreateInfo));
		hasher.Hash((const char*)&DepthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo));
		hasher.Hash((const char*)&ColorBlendingState, sizeof(VkPipelineColorBlendStateCreateInfo));

		return hasher.GetValue();
	}

	namespace Pipelines
	{
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts(VkDevice device, GraphicsPipelineDescriptor& descriptor)
		{
			return std::vector<VkDescriptorSetLayout>{};

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

		VkPipelineLayout GetPipelineLayout(VkDevice device, GraphicsPipelineDescriptor& descriptor)
		{
			return VK_NULL_HANDLE;

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

		static std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageInfo(VkDevice device, GraphicsPipelineDescriptor& descriptor)
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
					continue; // shader stage is not included

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

		static std::tuple<VkPipelineColorBlendStateCreateInfo, std::vector<VkPipelineColorBlendAttachmentState>> GetColorBlendState()
		{
			// Store these on the render pass attachments
			VkPipelineColorBlendAttachmentState colorBlendAttachment =
			{
				.blendEnable = VK_FALSE,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
								  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			};

			// Store this in the pipeline state, but override some parts
			VkPipelineColorBlendStateCreateInfo colorBlending =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.logicOpEnable = VK_FALSE,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachment,
			};

			return std::make_tuple(colorBlending, std::vector<VkPipelineColorBlendAttachmentState>{ colorBlendAttachment });
		}

		static VkPipeline CreateGraphicsPipeline(
			VkDevice device,
			GraphicsPipelineDescriptor& descriptor,
			VkRenderPass renderPass,
			uint32_t subpassIndex,
			VkViewport& viewport,
			VkRect2D& scissor)
		{
			return VK_NULL_HANDLE;

			auto shaderStages = GetPipelineShaderStageInfo(device, descriptor);
			auto pipelineLayout = GetPipelineLayout(device, descriptor);
			
			VkPipelineVertexInputStateCreateInfo vertexInputInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount = (uint32_t)descriptor.VertexInputBindings.size(),
				.pVertexBindingDescriptions = descriptor.VertexInputBindings.data(),
				.vertexAttributeDescriptionCount = (uint32_t)descriptor.VertexInputAttributes.size(),
				.pVertexAttributeDescriptions = descriptor.VertexInputAttributes.data(),
			};
			
			VkPipelineViewportStateCreateInfo viewportState =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.viewportCount = 1,
				.pViewports = &viewport,
				.scissorCount = 1,
				.pScissors = &scissor,
			};
			
			auto [colorBlendState, colorBlendAttachmentState] = GetColorBlendState();
			
			VkGraphicsPipelineCreateInfo pipelineInfo =
			{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount = (uint32_t)shaderStages.size(), // Number of shader stages
				.pStages = shaderStages.data(),
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &descriptor.InputAssemby,
				.pViewportState = &viewportState,
				.pRasterizationState = &descriptor.RasterizationState,
				.pMultisampleState = &descriptor.MultisamplingState,
				.pDepthStencilState = &descriptor.DepthStencilState, // Optional, null if not used
				.pColorBlendState = &colorBlendState,
				.layout = pipelineLayout,
				.renderPass = renderPass,
				.subpass = subpassIndex,
			};
			
			VkPipeline pipeline;
			auto result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
			
			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR(std::string("Failed to create pipeline for shader: ") + descriptor.Shader);
			
			return pipeline;
		}

		std::unordered_map<uint64_t, VkPipeline> g_Pipelines{};

		VkPipeline GetGraphicsPipeline(
			VkDevice device,
			GraphicsPipelineDescriptor& descriptor,
			VkRenderPass renderPass,
			uint32_t subpassIndex,
			VkViewport viewport,
			VkRect2D scissor)
		{
			auto hash = descriptor.Hash();

			auto pipelineSearch = g_Pipelines.find(hash);
			if (pipelineSearch != g_Pipelines.end())
				return pipelineSearch->second;

			g_Pipelines.emplace(hash, CreateGraphicsPipeline(device, descriptor, renderPass, subpassIndex, viewport, scissor));
			
			return g_Pipelines.at(hash);
		}
	}
}