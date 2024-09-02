#pragma once

#include <functional>
#include "vma_usage.h"

namespace Pixelate
{
	enum class FenceIdenfitier : uint32_t
	{
		FrameHasBeenPresented = 1,
		SwapchainLayoutTransition = 2,
		RenderGraphQueueSubmit = 4,
	};

	struct FenceGroupDescriptor
	{
		FenceIdenfitier Identifier;
		uint32_t FenceGroupSize;
		VkFenceCreateFlags CreateFlags = 0;

		uint64_t Hash() const;
	};
	
	class FenceGroup
	{
	public:
		FenceGroup(VkDevice device, const FenceGroupDescriptor& descriptor);
		void AddFencedCallback(std::function<void()> callback, uint32_t index = std::numeric_limits<uint32_t>::max());
		void Wait(uint32_t index = std::numeric_limits<uint32_t>::max(), uint64_t timeout = std::numeric_limits<uint64_t>::max());
		void Dispose();
		VkFence operator[](size_t index) { return m_VkFences[index]; }

	private:
		std::vector<VkFence> m_VkFences;
		std::unordered_map<uint32_t, std::vector<std::function<void()>>> m_IndexedCallbacks;
		VkDevice m_Device;
	};
}

namespace Pixelate::FenceManager
{
	FenceGroup& GetFenceGroup(VkDevice device, FenceGroupDescriptor descriptor);
	void Dispose();
}

