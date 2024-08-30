#pragma once

#include "vma_usage.h"
#include "pixelate_device.h"

namespace Pixelate
{
	enum class GraphicsQueueType : uint32_t
	{
		Default = 0,
	};

	struct GraphicsQueueSubmitDescriptor
	{
		GraphicsQueueType Type;
		uint64_t Hash(VkDevice device);
	};

	enum class ComputeQueueType : uint32_t
	{
		Default = 0,
	};

	struct ComputeQueueSubmitDescriptor
	{
		ComputeQueueType Type;
		uint64_t Hash(VkDevice device);
	};

	namespace QueueManager
	{
		void GraphicsQueueSubmit(
			PixelateDevice& device,
			GraphicsQueueSubmitDescriptor descriptor,
			VkCommandBuffer commandBuffer,
			VkFence signalFence,
			VkSemaphoreSubmitInfo* pSignalSemaphores,
			uint32_t signalSemaphoreCount,
			VkSemaphoreSubmitInfo* pWaitSemaphores,
			uint32_t waitSemaphoreCount);

		void GraphicsQueueSubmit(
			PixelateDevice& device,
			GraphicsQueueSubmitDescriptor descriptor,
			std::vector<VkCommandBuffer> commandBuffers,
			VkFence signalFence,
			VkSemaphoreSubmitInfo* pSignalSemaphores,
			uint32_t signalSemaphoreCount,
			VkSemaphoreSubmitInfo* pWaitSemaphores,
			uint32_t waitSemaphoreCount);

	}
}
