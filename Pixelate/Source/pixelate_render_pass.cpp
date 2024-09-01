#include "pixelate_render_pass.h"
#include "hasher.h"

namespace Pixelate
{
	uint64_t GraphicsPipelineDescriptor::Hash() const
	{
		Hasher hasher;

		hasher.Hash((const char*)DescriptorSetLayoutBindings.data(), sizeof(VkDescriptorSetLayoutBinding) * DescriptorSetLayoutBindings.size());
		hasher.Hash((const char*)PushConstantRanges.data(), sizeof(PushConstantRanges) * PushConstantRanges.size());
		hasher.Hash((const char*)VertexInputBindings.data(), sizeof(VkVertexInputBindingDescription) * VertexInputBindings.size());
		hasher.Hash((const char*)VertexInputAttributes.data(), sizeof(VkVertexInputAttributeDescription) * VertexInputAttributes.size());

		hasher.Hash((const char*)&ShaderDescriptor, sizeof(PixelateShaderDescriptor));
		hasher.Hash((const char*)&InputAssemby, sizeof(VkPipelineInputAssemblyStateCreateInfo));
		hasher.Hash((const char*)&RasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo));
		hasher.Hash((const char*)&MultisamplingState, sizeof(VkPipelineMultisampleStateCreateInfo));
		hasher.Hash((const char*)&DepthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo));
		hasher.Hash((const char*)&ColorBlendingState, sizeof(VkPipelineColorBlendStateCreateInfo));

		return hasher.GetValue();
	}

	PixelatePass::PixelatePass() :
		PassType(Pixelate::PassType::None),
		Name("default"),
		Flags(0),
		CommandBufferNone(nullptr),
		NoPipeline({}),
		Inputs({}),
		Outputs({})
	{
	}

	PixelatePass::PixelatePass(const PixelatePass& other) :
		PassType(other.PassType),
		Name(other.Name),
		Flags(other.Flags),
		Inputs(std::vector<PixelateResourceUsage>(other.Inputs)),
		Outputs(std::vector<PixelateResourceUsage>(other.Outputs))
	{
		switch (other.PassType)
		{
		case PassType::Graphics:
			new (&GraphicsPipelineDescriptor) Pixelate::GraphicsPipelineDescriptor(other.GraphicsPipelineDescriptor);
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			new (&HostPipelineDescriptor) Pixelate::HostPipelineDescriptor(other.HostPipelineDescriptor);
			CommandBufferHost = other.CommandBufferHost;
			break;
		}
	}

	PixelatePass& PixelatePass::operator=(const PixelatePass& other)
	{
		PassType = other.PassType;
		Name = other.Name;
		Flags = other.Flags;
		Inputs = std::vector<PixelateResourceUsage>(other.Inputs);
		Outputs = std::vector<PixelateResourceUsage>(other.Outputs);

		switch (other.PassType)
		{
		case PassType::Graphics:
			new (&GraphicsPipelineDescriptor) Pixelate::GraphicsPipelineDescriptor(other.GraphicsPipelineDescriptor);
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			new (&HostPipelineDescriptor) Pixelate::HostPipelineDescriptor(other.HostPipelineDescriptor);
			CommandBufferHost = other.CommandBufferHost;
			break;
		}

		return *this;
	}

	PixelatePass::PixelatePass(PixelatePass&& other) noexcept :
		PassType(other.PassType),
		Name(other.Name),
		Flags(other.Flags),
		Inputs(std::move(other.Inputs)),
		Outputs(std::move(other.Outputs))
	{
		switch (other.PassType)
		{
		case PassType::Graphics:
			new (&GraphicsPipelineDescriptor) Pixelate::GraphicsPipelineDescriptor(std::move(other.GraphicsPipelineDescriptor));
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			new (&HostPipelineDescriptor) Pixelate::HostPipelineDescriptor(std::move(other.HostPipelineDescriptor));
			CommandBufferHost = other.CommandBufferHost;
			break;
		}
	}

	PixelatePass& PixelatePass::operator=(PixelatePass&& other) noexcept
	{
		PassType = other.PassType;
		Name = other.Name;
		Flags = other.Flags;
		Inputs = std::move(other.Inputs);
		Outputs = std::move(other.Outputs);

		switch (other.PassType)
		{
		case PassType::Graphics:
			new (&GraphicsPipelineDescriptor) Pixelate::GraphicsPipelineDescriptor(std::move(other.GraphicsPipelineDescriptor));
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			new (&HostPipelineDescriptor) Pixelate::HostPipelineDescriptor(std::move(other.HostPipelineDescriptor));
			CommandBufferHost = other.CommandBufferHost;
			break;
		}

		return *this;
	}

	PixelatePass::PixelatePass(
		const char* name,
		Pixelate::GraphicsPipelineDescriptor&& pipeline,
		PixelatePassFlags flags,
		CommandGraphics commandBuffer,
		std::vector<PixelateResourceUsage>&& inputs,
		std::vector<PixelateResourceUsage>&& outputs) :
		PassType(Pixelate::PassType::Graphics),
		Name(name),
		GraphicsPipelineDescriptor(std::move(pipeline)),
		Flags(flags),
		CommandBufferGraphics(commandBuffer),
		Inputs(std::move(inputs)),
		Outputs(std::move(outputs))
	{
	}

	PixelatePass::~PixelatePass()
	{
		switch (PassType)
		{
		case PassType::Graphics:
			GraphicsPipelineDescriptor.~GraphicsPipelineDescriptor();
			break;
		}
	};
}