#include "command_buffer_manager.h"

namespace Pixelate
{
	uint64_t CommandBufferDescriptor::Hash(VkDevice device)
	{
		auto threadId = std::this_thread::get_id();

		Hasher hasher;

		hasher.Hash(std::hash<std::thread::id>()(threadId));
		hasher.Hash((uint32_t)Type);
		hasher.Hash((uint32_t)Level);
		hasher.Hash((uint32_t)PerformanceProfile);
		hasher.Hash((uint32_t)device);

		return hasher.GetValue();
	}

	void PixelateVkCommandBuffer::Return()
	{
		CommandBufferManager::ReturnCommandBuffer(Device, CommandBuffer, Descriptor);
	}
}

namespace Pixelate::CommandBufferManager
{
	std::unordered_map<uint64_t, VkCommandPool> g_CommandPools;
	std::unordered_map<uint64_t, ThreadSafeFifoQueue<VkCommandBuffer>> g_CommandBuffers;
	uint32_t g_LastCommandBufferAllocationCount = 8;
	static constexpr uint32_t s_CommandBufferGrowthRate = 2;

	static inline uint32_t GetQueueFamilyIndexFromBufferType(CommandBufferType type, QueueFamilyIndices queueFamilyIndices)
	{
		switch (type)
		{
		case CommandBufferType::GraphicsQueue:
			return queueFamilyIndices.GraphicsQueueFamily.value();
		case CommandBufferType::ComputeQueue:
			return queueFamilyIndices.ComputeQueueFamily.value();
		}

		return std::numeric_limits<uint32_t>::max();
	}

	static void AllocateCommandPool(VkDevice device, QueueFamilyIndices queueFamilyIndices, CommandBufferType type, VkCommandPool* commandPool)
	{
		uint32_t queueFamilyIndex = GetQueueFamilyIndexFromBufferType(type, queueFamilyIndices);

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndex;
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		vkCreateCommandPool(device, &createInfo, nullptr, commandPool);
	}

	static std::vector<VkCommandBuffer> AllocateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count)
	{
		std::vector<VkCommandBuffer> newCommandBuffers(count);
		
		VkCommandBufferAllocateInfo allocationInfo{};
		allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocationInfo.commandBufferCount = count;
		allocationInfo.commandPool = commandPool;
		allocationInfo.level = level;

		vkAllocateCommandBuffers(device, &allocationInfo, newCommandBuffers.data());

		return newCommandBuffers;
	}

	PixelateVkCommandBuffer CommandBufferManager::GetCommandBuffer(PixelateDevice device, CommandBufferDescriptor descriptor)
	{
		auto poolHash = descriptor.Hash(device.VkDevice);

		if (g_CommandBuffers.find(poolHash) != g_CommandBuffers.end())
		{
			VkCommandBuffer commandBuffer{};
			if (g_CommandBuffers[poolHash].try_pop(commandBuffer))
				return { device.VkDevice, descriptor, commandBuffer };
		}

		if (g_CommandPools.find(poolHash) == g_CommandPools.end())
		{
			g_CommandPools.insert({ poolHash, VK_NULL_HANDLE });
			AllocateCommandPool(device.VkDevice, device.QueueFamilyIndices, descriptor.Type, &g_CommandPools.at(poolHash));
		}

		auto bufferAllocationCount = g_LastCommandBufferAllocationCount * s_CommandBufferGrowthRate;
		g_LastCommandBufferAllocationCount = bufferAllocationCount;
		auto newCommandBuffers = AllocateCommandBuffers(device.VkDevice, g_CommandPools[poolHash], descriptor.Level, bufferAllocationCount);

		g_CommandBuffers[poolHash].push_range(newCommandBuffers);

		return { device.VkDevice, descriptor, g_CommandBuffers[poolHash].pop() };
	}

	void ReturnCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, CommandBufferDescriptor descriptor)
	{
		auto resetFlags = descriptor.PerformanceProfile == CommandBufferPerformanceProfile::PersistentResources
			? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
			: 0;

		auto hash = descriptor.Hash(device);
		vkResetCommandBuffer(commandBuffer, resetFlags);
		g_CommandBuffers[hash].push(commandBuffer);
	}
}