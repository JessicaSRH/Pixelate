#include "semaphore_manager.h"

namespace Pixelate
{

	uint64_t SemaphoreDescriptor::Hash() const
	{
		Hasher hasher;

		hasher.Hash((uint32_t)Identifier);
		hasher.Hash(Index);

		return hasher.GetValue();
	}

	namespace SemaphoreManager
	{
		std::unordered_map<uint64_t, PixelateSemaphore> m_SemaphoreGroups;

		PixelateSemaphore GetSemaphore(VkDevice device, VkPipelineStageFlags2 stageMask, SemaphoreDescriptor descriptor)
		{
			auto hash = descriptor.Hash();

			auto semaphoreFind = m_SemaphoreGroups.find(hash);

			if (semaphoreFind != m_SemaphoreGroups.end())
				return semaphoreFind->second;

			m_SemaphoreGroups.insert(std::make_pair(hash, PixelateSemaphore{ VK_NULL_HANDLE, stageMask }));

			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			vkCreateSemaphore(device, &createInfo, nullptr, &m_SemaphoreGroups[hash].Semaphore);

			return m_SemaphoreGroups.at(hash);
		}

		void Dispose(VkDevice device)
		{
			for (const auto& [hash, pixelateSemaphore] : m_SemaphoreGroups)
					vkDestroySemaphore(device, pixelateSemaphore.Semaphore, nullptr);
		}

		std::vector<VkSemaphoreSubmitInfo> GetSubmitSemaphores(PixelateSemaphore semaphore)
		{
			return std::vector<VkSemaphoreSubmitInfo>{ (VkSemaphoreSubmitInfo)semaphore };
		}
		
		std::vector<VkSemaphoreSubmitInfo> GetSubmitSemaphores(std::vector<PixelateSemaphore> semaphores)
		{
			std::vector<VkSemaphoreSubmitInfo> result{};
			result.reserve(semaphores.size());

			for (auto& semaphore : semaphores)
				result.push_back((VkSemaphoreSubmitInfo)semaphore);

			return result;
		}
	}

	PixelateSemaphore::operator VkSemaphore()
	{
		return Semaphore;
	}

	PixelateSemaphore::operator VkSemaphoreSubmitInfo()
	{
		VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
		semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		semaphoreSubmitInfo.deviceIndex = 0xFFFFFFFF;
		semaphoreSubmitInfo.semaphore = Semaphore;
		semaphoreSubmitInfo.stageMask = StageMask;
		semaphoreSubmitInfo.value = 0;
		return semaphoreSubmitInfo;
	}
}