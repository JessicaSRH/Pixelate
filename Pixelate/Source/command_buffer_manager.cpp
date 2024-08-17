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
		hasher.Hash((uint32_t)device);

		return hasher.GetValue();
	}

	uint64_t CommandBufferDescriptor::HashType(VkDevice device)
	{
		auto threadId = std::this_thread::get_id();

		Hasher hasher;

		hasher.Hash(std::hash<std::thread::id>()(threadId));
		hasher.Hash((uint32_t)Type);
		hasher.Hash((uint32_t)device);

		return hasher.GetValue();
	}
}

namespace Pixelate::CommandBufferManager
{

	std::unordered_map<uint64_t, VkCommandPool> m_CommandPools;
	std::unordered_map<uint64_t, ThreadSafeQueue<VkCommandBuffer>> m_CommandBuffers;
	uint32_t LastCommandBufferAllocationSize = 16;
	static constexpr uint32_t CommandBufferGrowthRate = 2;

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
		std::vector<VkCommandBuffer> newCommandBuffers{};

		VkCommandBufferAllocateInfo allocationInfo{};
		allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocationInfo.commandBufferCount = count;
		allocationInfo.commandPool = commandPool;
		allocationInfo.level = level;

		vkAllocateCommandBuffers(device, &allocationInfo, newCommandBuffers.data());

		return newCommandBuffers;
	}

	VkCommandBuffer CommandBufferManager::GetCommandBuffer(PixelateDevice device, CommandBufferDescriptor descriptor)
	{
		auto hash = descriptor.Hash(device.VkDevice);
		auto typeHash = descriptor.HashType(device.VkDevice);

		if (m_CommandBuffers.find(hash) != m_CommandBuffers.end())
		{
			VkCommandBuffer commandBuffer{};
			if (m_CommandBuffers[hash].try_pop(commandBuffer))
				return commandBuffer;
		}

		if (m_CommandPools.find(typeHash) == m_CommandPools.end())
		{
			m_CommandPools.insert({ typeHash, VK_NULL_HANDLE });
			AllocateCommandPool(device.VkDevice, device.QueueFamilyIndices, descriptor.Type, &m_CommandPools[hash]);
		}

		auto newCommandBuffers = AllocateCommandBuffers(device.VkDevice, m_CommandPools[typeHash], descriptor.Level, LastCommandBufferAllocationSize * CommandBufferGrowthRate);

		m_CommandBuffers[hash].push_range(newCommandBuffers);

		return m_CommandBuffers[hash].pop();
	}
}