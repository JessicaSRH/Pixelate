#pragma once

#include "internal_pixelate_include.h"

namespace Pixelate
{
	enum class CommandBufferType : uint32_t
	{
		GraphicsQueue,
		ComputeQueue,
	};

	enum class CommandBufferPerformanceProfile : uint32_t
	{
		Default = 0,
		PersistentResources = 1,
	};

	struct CommandBufferDescriptor
	{
		CommandBufferType Type = CommandBufferType::GraphicsQueue;
		VkCommandBufferLevel Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferPerformanceProfile PerformanceProfile = CommandBufferPerformanceProfile::Default;

		uint64_t Hash(VkDevice device); // thread-dependent hash!
	};

	struct PixelateVkCommandBuffer
	{
		VkDevice Device;
		CommandBufferDescriptor Descriptor;
		VkCommandBuffer CommandBuffer;

		operator VkCommandBuffer() const { return CommandBuffer; }
		void Return();
	};

	namespace CommandBufferManager
	{
		PixelateVkCommandBuffer GetCommandBuffer(PixelateDevice device, CommandBufferDescriptor descriptor = CommandBufferDescriptor());
		void ReturnCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, CommandBufferDescriptor descriptor = CommandBufferDescriptor());
	}
}
