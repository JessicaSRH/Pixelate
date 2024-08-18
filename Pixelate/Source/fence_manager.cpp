#include "fence_manager.h"

namespace Pixelate
{
	uint64_t FenceGroupDescriptor::Hash() const
	{
		Hasher hasher;

		hasher.Hash((uint32_t)Identifier);
		hasher.Hash(FenceGroupSize);
		hasher.Hash((uint32_t)CreateFlags);

		return hasher.GetValue();
	}

	void FenceGroup::AddFencedCallback(std::function<void()> callback, uint32_t index)
	{
		m_IndexedCallbacks[index].push_back(callback);
	}

	void FenceGroup::Wait(uint32_t index, uint64_t timeout)
	{
		if (index == std::numeric_limits<uint32_t>::max())
		{
			vkWaitForFences(m_Device, m_VkFences.size(), m_VkFences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());

			for (auto& [index, callbacks] : m_IndexedCallbacks)
			{
				for (const auto& callback : callbacks)
					callback();

				callbacks.clear();
			}

			vkResetFences(m_Device, m_VkFences.size(), m_VkFences.data());
		}
		else
		{
			vkWaitForFences(m_Device, 1, &m_VkFences[index], VK_TRUE, std::numeric_limits<uint64_t>::max());

			for (const auto& callback : m_IndexedCallbacks[index])
				callback();

			m_IndexedCallbacks[index].clear();

			vkResetFences(m_Device, 1, &m_VkFences[index]);
		}
	}

	void FenceGroup::Dispose()
	{
		for (const auto& fence : m_VkFences)
			vkDestroyFence(m_Device, fence, nullptr);
	}

	FenceGroup::FenceGroup(VkDevice device, const FenceGroupDescriptor& descriptor) : m_Device(device)
	{
		m_VkFences.resize(descriptor.FenceGroupSize);
		for (auto i = 0; i < descriptor.FenceGroupSize; i++)
		{
			VkFenceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			createInfo.flags = descriptor.CreateFlags;

			auto result = vkCreateFence(device, &createInfo, nullptr, &m_VkFences[i]);

			if (result != VK_SUCCESS)
				PXL8_CORE_ERROR("Failed to create fence!");
		}
	}
}

namespace Pixelate::FenceManager
{
	std::unordered_map<uint64_t, FenceGroup> g_FenceGroups;

	FenceGroup& GetFenceGroup(VkDevice device, FenceGroupDescriptor descriptor)
	{
		auto hash = descriptor.Hash();

		if (g_FenceGroups.find(hash) == g_FenceGroups.end())
			g_FenceGroups.emplace(std::make_pair(hash, FenceGroup(device, descriptor)));

		return g_FenceGroups.at(hash);
	}

	void Dispose()
	{
		for (auto& [descriptor, fenceGroup] : g_FenceGroups)
			fenceGroup.Dispose();
	}

}

