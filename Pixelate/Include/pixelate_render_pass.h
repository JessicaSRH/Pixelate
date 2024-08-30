#pragma once

#include "pipeline_manager.h"

namespace Pixelate
{
	typedef enum PixelateCommandBufferFlagBits : size_t
	{
		PIXELATE_COMMAND_BUFFER_RECORD_EVERY_FRAME = 0, // explicit definition to communicate intent; 
		PIXELATE_COMMAND_BUFFER_RECORD_ONCE = 1, // only  record the command buffer once; execute every frame
	} PixelateCommandBufferFlagBits;
	typedef size_t PixelateCommandBufferFlags;

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

	struct PixelatePass
	{
	public:
		PassType PassType;
		const char* Name;
		PixelateCommandBufferFlags Flags;

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

		PixelatePass();
		PixelatePass(const PixelatePass& other);
		PixelatePass(PixelatePass&& other) noexcept;
		PixelatePass& operator=(PixelatePass&& other) noexcept;
		PixelatePass(const char* name, Pixelate::GraphicsPipelineDescriptor pipeline, PixelateCommandBufferFlags flags, CommandGraphics commandBuffer);
		~PixelatePass();
		// inputs
		// outputs
	};
}
