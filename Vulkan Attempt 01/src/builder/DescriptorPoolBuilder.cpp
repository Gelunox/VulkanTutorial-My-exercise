#include "DescriptorPoolBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef DescriptorPoolBuilder::This This;

DescriptorPoolBuilder::DescriptorPoolBuilder(VkDevice& device): device(device)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.maxSets = 1;
}

This DescriptorPoolBuilder::addPoolSize(VkDescriptorType type, uint32_t count)
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = type;
	poolSize.descriptorCount = count;

	poolSizes.push_back( poolSize );

	return *this;
}

VkDescriptorPool DescriptorPoolBuilder::build()
{
	createInfo.poolSizeCount = poolSizes.size();
	createInfo.pPoolSizes = poolSizes.data();

	VkDescriptorPool pool;
	if (vkCreateDescriptorPool( device, &createInfo, nullptr, &pool ) != VK_SUCCESS)
	{
		throw runtime_error( "Descriptorpool creation failed" );
	}

	return pool;
}