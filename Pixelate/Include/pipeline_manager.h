#pragma once

#include "vma_usage.h"

namespace Pixelate
{
	
	struct GraphicsPipelineDescriptor
	{
	public:
		static constexpr VkShaderStageFlags DefaultShaderStageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		static constexpr VkPipelineInputAssemblyStateCreateInfo DefaultInputAssembly = VkPipelineInputAssemblyStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		static constexpr VkPipelineRasterizationStateCreateInfo DefaultRasterizationState
		{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.depthClampEnable = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode = VK_POLYGON_MODE_FILL,
				.cullMode = VK_CULL_MODE_BACK_BIT,
				.frontFace = VK_FRONT_FACE_CLOCKWISE,
				.depthBiasEnable = VK_FALSE,
				.lineWidth = 1.0f,
		};
		static constexpr VkPipelineMultisampleStateCreateInfo DefaultMultisamplingState =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 0.0f,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};
		static constexpr VkPipelineDepthStencilStateCreateInfo DefaultDepthStencilState =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
		};
		static constexpr VkPipelineColorBlendStateCreateInfo DefaultColorBlendingState =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.attachmentCount = 0, // this get's overriden during pipeline creation, do not set manually
			.pAttachments = nullptr, // this get's overriden during pipeline creation, do not set manually
		};

		const char* Shader = "default";
		VkShaderStageFlags ShaderStageFlags;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> DescriptorSetLayoutBindings = {};
		std::vector<VkPushConstantRange> PushConstantRanges = {};
		std::vector<VkVertexInputBindingDescription> VertexInputBindings = {};
		std::vector< VkVertexInputAttributeDescription> VertexInputAttributes = {};
		VkPipelineInputAssemblyStateCreateInfo InputAssemby = DefaultInputAssembly;
		VkPipelineRasterizationStateCreateInfo RasterizationState = DefaultRasterizationState;
		VkPipelineMultisampleStateCreateInfo MultisamplingState = DefaultMultisamplingState;
		VkPipelineDepthStencilStateCreateInfo DepthStencilState = DefaultDepthStencilState;
		VkPipelineColorBlendStateCreateInfo ColorBlendingState = DefaultColorBlendingState;

	public:

		GraphicsPipelineDescriptor(
			const char* shader,
			std::vector<std::vector<VkDescriptorSetLayoutBinding>>&& descriptorSetLayoutBindings = {},
			std::vector<VkPushConstantRange>&& pushConstantRanges = {},
			std::vector<VkVertexInputBindingDescription>&& vertexInputBindings = {},
			std::vector< VkVertexInputAttributeDescription>&& vertexInputAttributes = {},
			VkShaderStageFlags shaderStageFlags = DefaultShaderStageFlags,
			VkPipelineInputAssemblyStateCreateInfo inputAssembly = DefaultInputAssembly,
			VkPipelineRasterizationStateCreateInfo rasterizationState = DefaultRasterizationState,
			VkPipelineMultisampleStateCreateInfo multisamplingState = DefaultMultisamplingState,
			VkPipelineDepthStencilStateCreateInfo depthStencilState = DefaultDepthStencilState,
			VkPipelineColorBlendStateCreateInfo colorBlendingState = DefaultColorBlendingState) :
			Shader(shader),
			ShaderStageFlags(shaderStageFlags),
			DescriptorSetLayoutBindings(descriptorSetLayoutBindings),
			PushConstantRanges(pushConstantRanges),
			VertexInputBindings(vertexInputBindings),
			VertexInputAttributes(vertexInputAttributes),
			InputAssemby(inputAssembly),
			RasterizationState(rasterizationState),
			MultisamplingState(multisamplingState),
			DepthStencilState(depthStencilState),
			ColorBlendingState(colorBlendingState)
		{
		}

		GraphicsPipelineDescriptor() = default;
		~GraphicsPipelineDescriptor() = default;
		GraphicsPipelineDescriptor(const GraphicsPipelineDescriptor& other);
		GraphicsPipelineDescriptor& operator=(const GraphicsPipelineDescriptor& other) noexcept;
		GraphicsPipelineDescriptor(GraphicsPipelineDescriptor&& other) noexcept;
		GraphicsPipelineDescriptor& operator=(GraphicsPipelineDescriptor&& other) noexcept;

		uint64_t Hash() const;
	};

	namespace Pipelines
	{
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts(VkDevice device, GraphicsPipelineDescriptor& descriptor);
		VkPipelineLayout GetPipelineLayout(VkDevice device, GraphicsPipelineDescriptor& descriptor);
		VkPipeline GetGraphicsPipeline(
			VkDevice device,
			GraphicsPipelineDescriptor& descriptor,
			VkRenderPass renderPass,
			uint32_t subpassIndex,
			VkViewport viewport,
			VkRect2D scissor);
	}
}