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

void MemoryFactory::createTextureImage( char * location, VkImage& dstImage, VkDeviceMemory& dstMemory )
{
	int texWidth,
		texHeight,
		texChannels;

	stbi_uc* pixels = stbi_load( location, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw runtime_error( "couldn't load image" );
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	createBuffer( imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingMemory );

	void* data;
	vkMapMemory( logicalDevice, stagingMemory, 0, imageSize, 0, &data );
	memcpy( data, pixels, (size_t)imageSize );
	vkUnmapMemory( logicalDevice, stagingMemory );

	stbi_image_free( pixels );

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(texWidth);
	imageInfo.extent.height = static_cast<uint32_t>(texHeight);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (vkCreateImage( logicalDevice, &imageInfo, nullptr, &dstImage ) != VK_SUCCESS)
	{
		throw runtime_error( "Image creation failed" );
	}

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements( logicalDevice, dstImage, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = Util::findMemoryType( physicalDevice, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	if (vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &dstMemory ) != VK_SUCCESS)
	{
		throw runtime_error( "Image memory creation failed" );
	}

	vkBindImageMemory( logicalDevice, dstImage, dstMemory, 0 );

	//these could be combined into a single commandbuffer
	transitionImageLayout( dstImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
	copyBufferToImage(stagingBuffer, dstImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) );
	transitionImageLayout( dstImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

	vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( logicalDevice, stagingMemory, nullptr );
}

void MemoryFactory::transitionImageLayout( VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout )
{
	VkCommandBuffer cmdBuffer = beginOneTimeUsageCommand();

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw invalid_argument( "unsupported transition layout" );
	}

	vkCmdPipelineBarrier( cmdBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier );

	endOneTimeUsageCommand( cmdBuffer );
}

void MemoryFactory::copyBufferToImage( VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height )
{
	VkCommandBuffer cmdBuffer = beginOneTimeUsageCommand();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage( cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

	endOneTimeUsageCommand( cmdBuffer );
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