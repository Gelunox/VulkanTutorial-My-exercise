#include "DescriptorSetLayoutBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef DescriptorSetLayoutBuilder::This This;

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDevice& device): device(device)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
}

This DescriptorSetLayoutBuilder::addBinding(uint32_t bindIndex, VkDescriptorType type, uint32_t count, VkShaderStageFlags stageFlags, VkSampler* immutableSamplers)
{
	VkDescriptorSetLayoutBinding binding = {};
	binding.binding = bindIndex;
	binding.descriptorType = type;
	binding.descriptorCount = count;
	binding.stageFlags = stageFlags;
	binding.pImmutableSamplers = immutableSamplers;

	bindings.push_back( binding );

	return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build()
{
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();

	VkDescriptorSetLayout layout;
	if (vkCreateDescriptorSetLayout( device, &createInfo, nullptr, &layout ) != VK_SUCCESS)
	{
		throw runtime_error( "Failed to create descriptorset layout" );
	}

	return layout;
}