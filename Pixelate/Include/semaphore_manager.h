#pragma once
#include "internal_pixelate_include.h"

namespace Pixelate
{
	enum class SemaphoreIdentifier : uint32_t
	{
		SwapchainImageHasBeenAcquired = 1,
		SwapchainImageTransitionToPresent = 2,
	};

	struct SemaphoreDescriptor
	{
		const SemaphoreIdentifier Identifier = SemaphoreIdentifier::SwapchainImageHasBeenAcquired;
		const uint32_t Index;

		uint64_t Hash() const;
	};

	class PixelateSemaphore
	{
	public:
		PixelateSemaphore(VkSemaphore semaphore = VK_NULL_HANDLE, VkPipelineStageFlags2 stageMask = VK_PIPELINE_STAGE_2_NONE);

		VkSemaphore Semaphore;
		VkPipelineStageFlags2 StageMask;
		VkSemaphoreSubmitInfo SemaphoreSubmitInfo;
		operator VkSemaphore();
	};

	namespace SemaphoreManager
	{
		PixelateSemaphore& GetSemaphore(VkDevice device,VkPipelineStageFlags2 stageMask, SemaphoreDescriptor descriptor);
		void Dispose(VkDevice device);
		std::vector<VkSemaphoreSubmitInfo> GetSemaphoreSubmitInfo(PixelateSemaphore semaphore);
		std::vector<VkSemaphoreSubmitInfo> GetSemaphoreSubmitInfos(std::vector<PixelateSemaphore> semaphores);
	}
}