#include <unordered_map>
#include "semaphore_manager.h"
#include "hasher.h"

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

		PixelateSemaphore& GetSemaphore(VkDevice device, VkPipelineStageFlags2 stageMask, SemaphoreDescriptor descriptor)
		{
			auto hash = descriptor.Hash();

			auto semaphoreFind = m_SemaphoreGroups.find(hash);

			if (semaphoreFind != m_SemaphoreGroups.end())
				return semaphoreFind->second;

			VkSemaphore semaphore{};
			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);

			m_SemaphoreGroups.insert(std::make_pair(hash, PixelateSemaphore(semaphore, stageMask)));

			return m_SemaphoreGroups.at(hash);
		}

		void Dispose(VkDevice device)
		{
			for (const auto& [hash, pixelateSemaphore] : m_SemaphoreGroups)
					vkDestroySemaphore(device, pixelateSemaphore.Semaphore, nullptr);
		}

		std::vector<VkSemaphoreSubmitInfo> GetSemaphoreSubmitInfo(PixelateSemaphore semaphore)
		{
			return std::vector<VkSemaphoreSubmitInfo>{ semaphore.SemaphoreSubmitInfo };
		}
		
		std::vector<VkSemaphoreSubmitInfo> GetSemaphoreSubmitInfos(std::vector<PixelateSemaphore> semaphores)
		{
			std::vector<VkSemaphoreSubmitInfo> result{};
			result.reserve(semaphores.size());

			for (auto& semaphore : semaphores)
				result.push_back(semaphore.SemaphoreSubmitInfo);

			return result;
		}
	}

	PixelateSemaphore::PixelateSemaphore(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask) :
		Semaphore(semaphore),
		StageMask(stageMask),
		SemaphoreSubmitInfo(VkSemaphoreSubmitInfo
			{
				VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				nullptr,
				Semaphore,
				0,
				StageMask,
				0b1
			})
	{
	}

	PixelateSemaphore::operator VkSemaphore()
	{
		return Semaphore;
	}
}