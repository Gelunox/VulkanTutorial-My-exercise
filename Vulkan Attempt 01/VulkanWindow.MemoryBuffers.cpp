#include "VulkanWindow.hpp"

//https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
//https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer

using namespace com::gelunox::vulcanUtils;
using namespace std;

void VulkanWindow::createVertexBuffers()
{
	VkDeviceSize bufferSize = sizeof( vertices[0] ) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	//staging buffer
	createBuffer( bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingMemory );

	void* data;
	vkMapMemory( logicalDevice, stagingMemory, 0, bufferSize, 0, &data );
	memcpy( data, vertices.data(), (size_t)bufferSize );
	vkUnmapMemory( logicalDevice, stagingMemory );

	//vertex buffer
	createBuffer( bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory );

	copyBuffer( stagingBuffer, vertexBuffer, bufferSize );

	vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( logicalDevice, stagingMemory, nullptr );
}

void VulkanWindow::createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property, VkBuffer &buffer, VkDeviceMemory &memory )
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
	allocInfo.memoryTypeIndex = findMemoryType( memReq.memoryTypeBits, property );

	if (vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &memory ) != VK_SUCCESS)
	{
		throw runtime_error( "could not allocate gpu memory" );
	}

	vkBindBufferMemory( logicalDevice, buffer, memory, 0 );
}

void VulkanWindow::copyBuffer( VkBuffer src, VkBuffer dst, VkDeviceSize size )
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandpool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	vkAllocateCommandBuffers( logicalDevice, &allocInfo, &cmdBuffer );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( cmdBuffer, &beginInfo );

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer( cmdBuffer, src, dst, 1, &copyRegion );
	vkEndCommandBuffer( cmdBuffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit( graphicsQ, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( graphicsQ );

	vkFreeCommandBuffers( logicalDevice, commandpool, 1, &cmdBuffer );
}
