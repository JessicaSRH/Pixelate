#pragma once

#include "vma_usage.h"

namespace Pixelate
{
	struct PixelateShaderDescriptor
	{
		const char* Name;
		const char* Path; // path to containing folder
		VkShaderStageFlags ShaderStages;
	};

	struct GraphicsPipelineDescriptor
	{
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

		PixelateShaderDescriptor ShaderDescriptor = {};
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> DescriptorSetLayoutBindings = {};
		std::vector<VkPushConstantRange> PushConstantRanges = {};
		std::vector<VkVertexInputBindingDescription> VertexInputBindings = {};
		std::vector<VkVertexInputAttributeDescription> VertexInputAttributes = {};
		VkPipelineInputAssemblyStateCreateInfo InputAssemby = DefaultInputAssembly;
		VkPipelineRasterizationStateCreateInfo RasterizationState = DefaultRasterizationState;
		VkPipelineMultisampleStateCreateInfo MultisamplingState = DefaultMultisamplingState;
		VkPipelineDepthStencilStateCreateInfo DepthStencilState = DefaultDepthStencilState;
		VkPipelineColorBlendStateCreateInfo ColorBlendingState = DefaultColorBlendingState;

		uint64_t Hash() const;
	};

	typedef enum PixelatePassFlagBits : size_t
	{
		PIXELATE_PASS_NO_FLAG = 0,
		PIXELATE_PASS_RECORD_ONCE = 1, // only  record the command buffer once; execute every frame
		PIXELATE_PASS_COLOR_ATTACHMENT_IS_SWAPCHAIN = 2,
	} PixelatePassFlagBits;
	typedef size_t PixelatePassFlags;

	enum class PassType : size_t
	{
		None = 0,
		Graphics = 1,
		Host = 2,
	};

	typedef void (*CommandGraphics)(VkCommandBuffer commandBuffer, VkPipeline pipelineHandle);
	typedef void (*CommandHost)();

	struct HostPipelineDescriptor
	{
		// empty for now, still declared to show the design principle
	};

	enum class PixelateResourceType : char
	{
		Image = 0,
		Buffer = 1,
	};

	struct PhysicalImageDescriptor
	{
		VkFormat Format;
		uint32_t Width;
		uint32_t Height;
	};

	struct PhysicalBufferDescriptor
	{
		size_t Size = 0;
	};

	struct PixelateResource
	{
		const char* Name;
		PixelateResourceType Type;
		union
		{
			// If Type == PixelateResourceType::SwapchainImage, no info is necessary
			PhysicalImageDescriptor PhysicalImageDescriptor;
			PhysicalBufferDescriptor PhysicalBufferDescriptor;
		};
		//std::vector<const char*> WrittenInPasses;
		//std::vector<const char*> ReadInPasses;
	};

	typedef enum PixelateResourceUsageFlags : uint64_t
	{
		PIXELATE_USAGE_NONE = 0,
		PIXELATE_USAGE_COLOR_ATTACMENT = 1,
		PIXELATE_USAGE_DEPTH_ATTACMENT = 2,
		PIXELATE_USAGE_INDEX_BUFFER = 4,
		PIXELATE_USAGE_VERTEX_BUFFER = 8,
		PIXELATE_USAGE_STORAGE_BUFFER = 16,
		PIXELATE_USAGE_SAMPLED_TEXTURE_BUFFER = 32,
		// etc.
	} PixelateResourceUsageTypeFlagBits;
	typedef uint64_t PixelateResourceUsageFlag;

	typedef enum PixelateResourceStageFlags : uint64_t
	{
		PIXELATE_USAGE_STAGE_HOST = VK_PIPELINE_STAGE_2_HOST_BIT,
		PIXELATE_USAGE_STAGE_VERTEX_INPUT = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
		PIXELATE_USAGE_STAGE_VERTEX_SHADER = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
		PIXELATE_USAGE_STAGE_FRAGMENT_SHADER = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
		PIXELATE_USAGE_STAGE_EARLY_DEPTH = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
		PIXELATE_USAGE_STAGE_LATE_DEPTH = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		PIXELATE_USAGE_STAGE_COMPUTE = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		PIXELATE_USAGE_STAGE_COPY = VK_PIPELINE_STAGE_2_COPY_BIT,
		PIXELATE_USAGE_STAGE_BLIT = VK_PIPELINE_STAGE_2_BLIT_BIT,
		PIXELATE_USAGE_STAGE_CLEAR = VK_PIPELINE_STAGE_2_CLEAR_BIT,
		PIXELATE_USAGE_STAGE_RESOLVE = VK_PIPELINE_STAGE_2_RESOLVE_BIT,
		// etc.
	} PixelateResourceUsageStageFlagBits;
	typedef uint64_t PixelateResourceUsageStageFlag;

	struct PixelateResourceUsage
	{
		PixelateResource Resource;
		PixelateResourceUsageFlag UsageFlags;
		PixelateResourceStageFlags StageFlags;
		VkPipelineColorBlendAttachmentState BlendState = // optional, ignored if resource is not an attachment
		{
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};
	};

	struct PixelatePass
	{
	public:
		PassType PassType;
		const char* Name;
		PixelatePassFlags Flags;

		union
		{
			struct {} NoPipeline;
			GraphicsPipelineDescriptor GraphicsPipelineDescriptor;
			HostPipelineDescriptor HostPipelineDescriptor;
		};
		union
		{
			void* CommandBufferNone;
			CommandGraphics CommandBufferGraphics;
			CommandHost CommandBufferHost;
		};

		std::vector<PixelateResourceUsage> Inputs;
		std::vector<PixelateResourceUsage> Outputs;

		PixelatePass();
		PixelatePass(const PixelatePass& other);
		PixelatePass& operator=(const PixelatePass& other);
		PixelatePass(PixelatePass&& other) noexcept;
		PixelatePass& operator=(PixelatePass&& other) noexcept;
		PixelatePass(const char* name, Pixelate::GraphicsPipelineDescriptor&& pipeline, PixelatePassFlags flags, CommandGraphics commandBuffer, std::vector<PixelateResourceUsage>&& inputs, std::vector<PixelateResourceUsage>&& outputs);
		~PixelatePass();
	};
}
