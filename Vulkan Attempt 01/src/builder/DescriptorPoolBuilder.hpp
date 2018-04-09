#pragma once

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class DescriptorPoolBuilder
	{
	public:
		typedef DescriptorPoolBuilder& This;
	private:
		VkDescriptorPoolCreateInfo createInfo = {};

		VkDevice device;

		vector<VkDescriptorPoolSize> poolSizes;
	public:
		DescriptorPoolBuilder(VkDevice& device);

		This addPoolSize( VkDescriptorType type, uint32_t count );

		VkDescriptorPool build();
	};
};