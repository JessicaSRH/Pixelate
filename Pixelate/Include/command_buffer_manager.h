#pragma once

#include "pixelate_include.h"

namespace Pixelate
{
	enum class CommandBufferType : uint32_t
	{
		GraphicsQueue,
		ComputeQueue,
	};

	struct CommandBufferDescriptor
	{
		CommandBufferType Type = CommandBufferType::GraphicsQueue;
		VkCommandBufferLevel Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		uint64_t Hash(VkDevice device); // thread-dependent hash!
		uint64_t HashType(VkDevice device); // thread-dependent hash!
	};

	namespace CommandBufferManager // Use namespace to get single-ton like functionality
	{
		VkCommandBuffer GetCommandBuffer(PixelateDevice device, CommandBufferDescriptor descriptor);
	}

}
