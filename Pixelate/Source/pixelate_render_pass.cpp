#include "pixelate_render_pass.h"

namespace Pixelate
{
	PixelatePass::PixelatePass() :
		PassType(Pixelate::PassType::None),
		Name("default"),
		Flags(0),
		CommandBufferNone(nullptr),
		NoPipeline({})
	{
	}

	PixelatePass::PixelatePass(const PixelatePass& other) :
		PassType(other.PassType),
		Name(other.Name),
		Flags(other.Flags),
		CommandBufferGraphics(nullptr),
		HostPipelineDescriptor({})
	{
		switch (other.PassType)
		{
		case PassType::Graphics:
			GraphicsPipelineDescriptor = other.GraphicsPipelineDescriptor;
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			HostPipelineDescriptor = other.HostPipelineDescriptor;
			CommandBufferHost = other.CommandBufferHost;
			break;
		}
	}

	PixelatePass::PixelatePass(PixelatePass&& other) noexcept :
		PassType(other.PassType),
		Name(other.Name),
		Flags(other.Flags),
		CommandBufferGraphics(nullptr),
		HostPipelineDescriptor({})
	{
		switch (other.PassType)
		{
		case PassType::Graphics:
			GraphicsPipelineDescriptor = std::move(other.GraphicsPipelineDescriptor);
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			HostPipelineDescriptor = other.HostPipelineDescriptor;
			CommandBufferHost = other.CommandBufferHost;
			break;
		}
	}

	PixelatePass& PixelatePass::operator=(PixelatePass&& other) noexcept
	{
		PassType = other.PassType;
		Name = other.Name;
		Flags = other.Flags;
		CommandBufferGraphics = nullptr;

		switch (other.PassType)
		{
		case PassType::Graphics:
			GraphicsPipelineDescriptor = std::move(other.GraphicsPipelineDescriptor);
			CommandBufferGraphics = other.CommandBufferGraphics;
			break;
		case PassType::Host:
			HostPipelineDescriptor = other.HostPipelineDescriptor;
			CommandBufferHost = other.CommandBufferHost;
			break;
		}

		return *this;
	}

	PixelatePass::PixelatePass(const char* name, Pixelate::GraphicsPipelineDescriptor pipeline, PixelateCommandBufferFlags flags, CommandGraphics commandBuffer) :
		PassType(Pixelate::PassType::Graphics),
		Name(name),
		GraphicsPipelineDescriptor(std::move(pipeline)),
		Flags(flags),
		CommandBufferGraphics(commandBuffer)
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