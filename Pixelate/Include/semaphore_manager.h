#pragma once
#include "pixelate_include.h"

namespace Pixelate
{
	enum class SemaphoreIdentifier : uint32_t
	{
		SwapchainImageHasBeenAcquired = 1,
		SwapchainImageTransitionToPresent = 2,
	};

	struct SemaphoreDescriptor
	{
		SemaphoreIdentifier Identifier = SemaphoreIdentifier::SwapchainImageHasBeenAcquired;
		uint32_t Index;

		uint64_t Hash() const;
	};

	struct PixelateSemaphore
	{
		VkSemaphore Semaphore = VK_NULL_HANDLE;
		VkPipelineStageFlags2 StageMask = VK_PIPELINE_STAGE_2_NONE;

		operator VkSemaphore();
		operator VkSemaphoreSubmitInfo();
	};

	namespace SemaphoreManager
	{
		PixelateSemaphore GetSemaphore(VkDevice device,VkPipelineStageFlags2 stageMask, SemaphoreDescriptor descriptor);
		void Dispose(VkDevice device);
		std::vector<VkSemaphoreSubmitInfo> GetSubmitSemaphores(PixelateSemaphore semaphore);
		std::vector<VkSemaphoreSubmitInfo> GetSubmitSemaphores(std::vector<PixelateSemaphore> semaphores);
	}
}