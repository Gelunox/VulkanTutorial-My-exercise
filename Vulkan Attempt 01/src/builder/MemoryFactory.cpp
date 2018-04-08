#include "MemoryFactory.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace com::gelunox::vulcanUtils;


MemoryFactory::MemoryFactory()
{
}


MemoryFactory::~MemoryFactory()
{
}

void MemoryFactory::createBufferMemory( VkDeviceSize size, void const* srcData, VkBuffer& dstBuffer, VkDeviceMemory& dstMemory, VkBufferUsageFlagBits flags )
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	createBuffer( size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingMemory );

	void* data;
	vkMapMemory( logicalDevice, stagingMemory, 0, size, 0, &data );
	memcpy( data, srcData, (size_t)size );
	vkUnmapMemory( logicalDevice, stagingMemory );

	createBuffer( size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		dstBuffer, dstMemory );

	copyBuffer( stagingBuffer, dstBuffer, size );

	vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( logicalDevice, stagingMemory, nullptr );
}

void MemoryFactory::createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property, VkBuffer &buffer, VkDeviceMemory &memory )
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer( logicalDevice, &bufferInfo, nullptr, &buffer ) != VK_SUCCESS)
	{
		throw runtime_error( "Error creating vertex buffer" );
	}

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements( logicalDevice, buffer, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = Util::findMemoryType( physicalDevice, memReq.memoryTypeBits, property );

	if (vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &memory ) != VK_SUCCESS)
	{
		throw runtime_error( "could not allocate gpu memory" );
	}

	vkBindBufferMemory( logicalDevice, buffer, memory, 0 );
}

void MemoryFactory::copyBuffer( VkBuffer src, VkBuffer dst, VkDeviceSize size )
{
	VkCommandBuffer cmdBuffer = beginOneTimeUsageCommand();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer( cmdBuffer, src, dst, 1, &copyRegion );

	endOneTimeUsageCommand( cmdBuffer );
}

VkCommandBuffer MemoryFactory::beginOneTimeUsageCommand()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	vkAllocateCommandBuffers( logicalDevice, &allocInfo, &cmdBuffer );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( cmdBuffer, &beginInfo );

	return cmdBuffer;
}

void MemoryFactory::endOneTimeUsageCommand( VkCommandBuffer& cmdBuffer )
{
	vkEndCommandBuffer( cmdBuffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit( copyQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( copyQueue );

	vkFreeCommandBuffers( logicalDevice, commandPool, 1, &cmdBuffer );
}