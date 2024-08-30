
#include <unordered_map>

#include "queue_manager.h"
#include "hasher.h"

namespace Pixelate
{
	uint64_t GraphicsQueueSubmitDescriptor::Hash(VkDevice device)
	{
		Hasher hasher{};

		hasher.Hash((uint32_t)Type);
		hasher.Hash((uint32_t)device);

		return hasher.GetValue();
	}
	uint64_t ComputeQueueSubmitDescriptor::Hash(VkDevice device)
	{
		Hasher hasher{};

		hasher.Hash((uint32_t)Type);
		hasher.Hash((uint32_t)device);

		return hasher.GetValue();
	}
	
	namespace QueueManager
	{
		std::unordered_map<uint64_t, VkQueue> g_Queues{};

		static void QueueSubmit1(
			PixelateDevice& device,
			GraphicsQueueSubmitDescriptor descriptor,
			VkCommandBuffer commandBuffer,
			VkFence signalFence,
			VkSemaphoreSubmitInfo* pSignalSemaphores,
			uint32_t signalSemaphoreCount,
			VkSemaphoreSubmitInfo* pWaitSemaphores,
			uint32_t waitSemaphoreCount)
		{
			auto hash = descriptor.Hash(device.VkDevice);
			auto& queue = g_Queues[hash];

			if (queue == VK_NULL_HANDLE)
				vkGetDeviceQueue(device.VkDevice, device.QueueFamilyIndices.GraphicsQueueFamily.value(), (uint32_t)descriptor.Type, &queue);

			std::vector<VkSemaphore> waitSemaphoresVector{};
			if (pWaitSemaphores)
				waitSemaphoresVector.push_back(pWaitSemaphores->semaphore);

			std::vector<VkSemaphore> signalSemaphoreVector{ pSignalSemaphores->semaphore };
			if (pSignalSemaphores)
				signalSemaphoreVector.push_back(pSignalSemaphores->semaphore);

			VkPipelineStageFlags dstWaitStages = VK_PIPELINE_STAGE_NONE;

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
			submitInfo.signalSemaphoreCount = signalSemaphoreVector.size();
			submitInfo.pSignalSemaphores = signalSemaphoreVector.data();
			submitInfo.waitSemaphoreCount = waitSemaphoresVector.size();
			submitInfo.pWaitSemaphores = waitSemaphoresVector.data();
			submitInfo.pWaitDstStageMask = &dstWaitStages;

			vkQueueSubmit(queue, 1, &submitInfo, signalFence);
		}

		void GraphicsQueueSubmit(
			PixelateDevice& device,
			GraphicsQueueSubmitDescriptor descriptor,
			VkCommandBuffer commandBuffer,
			VkFence signalFence,
			VkSemaphoreSubmitInfo* pSignalSemaphores,
			uint32_t signalSemaphoreCount,
			VkSemaphoreSubmitInfo* pWaitSemaphores,
			uint32_t waitSemaphoreCount)
		{
			auto hash = descriptor.Hash(device.VkDevice);
			auto& queue = g_Queues[hash];

			if (queue == VK_NULL_HANDLE)
				vkGetDeviceQueue(device.VkDevice, device.QueueFamilyIndices.GraphicsQueueFamily.value(), (uint32_t)descriptor.Type, &queue);

			VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
			commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandBufferSubmitInfo.commandBuffer = commandBuffer;
			commandBufferSubmitInfo.deviceMask = 01;

			VkSubmitInfo2 queueSubmitInfo{};
			queueSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
			queueSubmitInfo.commandBufferInfoCount = 1;
			queueSubmitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;
			queueSubmitInfo.signalSemaphoreInfoCount = signalSemaphoreCount;
			queueSubmitInfo.pSignalSemaphoreInfos = pSignalSemaphores;
			queueSubmitInfo.waitSemaphoreInfoCount = waitSemaphoreCount;
			queueSubmitInfo.pWaitSemaphoreInfos = pWaitSemaphores;

			vkQueueSubmit2(queue, 1, &queueSubmitInfo, signalFence);
		}

		void GraphicsQueueSubmit(
			PixelateDevice& device,
			GraphicsQueueSubmitDescriptor descriptor,
			std::vector<VkCommandBuffer> commandBuffers,
			VkFence signalFence,
			VkSemaphoreSubmitInfo* pSignalSemaphores,
			uint32_t signalSemaphoreCount,
			VkSemaphoreSubmitInfo* pWaitSemaphores,
			uint32_t waitSemaphoreCount)
		{
			auto hash = descriptor.Hash(device.VkDevice);
			auto& queue = g_Queues[hash];

			if (queue == VK_NULL_HANDLE)
				vkGetDeviceQueue(device.VkDevice, device.QueueFamilyIndices.GraphicsQueueFamily.value(), (uint32_t)descriptor.Type, &queue);

			std::vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos{};
			commandBufferSubmitInfos.resize(commandBuffers.size());

			for (const auto& commandBuffer : commandBuffers)
			{
				commandBufferSubmitInfos.emplace_back(VkCommandBufferSubmitInfo
					{
						VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
						nullptr,
						commandBuffer,
						0xFFFFFFFF
					});
			}

			VkSubmitInfo2 queueSubmitInfo{};
			queueSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
			queueSubmitInfo.commandBufferInfoCount = commandBufferSubmitInfos.size();
			queueSubmitInfo.pCommandBufferInfos = commandBufferSubmitInfos.data();
			queueSubmitInfo.signalSemaphoreInfoCount = signalSemaphoreCount;
			queueSubmitInfo.pSignalSemaphoreInfos = pSignalSemaphores;
			queueSubmitInfo.waitSemaphoreInfoCount = waitSemaphoreCount;
			queueSubmitInfo.pWaitSemaphoreInfos = pWaitSemaphores;

			vkQueueSubmit2(queue, 1, &queueSubmitInfo, signalFence);
		}
	}
}

