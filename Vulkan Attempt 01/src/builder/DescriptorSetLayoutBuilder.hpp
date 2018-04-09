#pragma once

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class DescriptorSetLayoutBuilder
	{
	public:
		typedef DescriptorSetLayoutBuilder& This;
	private:
		VkDescriptorSetLayoutCreateInfo createInfo = {};

		VkDevice device;

		vector<VkDescriptorSetLayoutBinding> bindings;
	public:
		DescriptorSetLayoutBuilder(VkDevice& device);

		This addBinding( uint32_t bindIndex, VkDescriptorType type, uint32_t count, VkShaderStageFlags stageFlags, VkSampler * immutableSamplers );

		VkDescriptorSetLayout build();
	};
};