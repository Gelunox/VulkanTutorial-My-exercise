#pragma once

#include <vulkan/vulkan.h>
#include "../util/Util.hpp"

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class MemoryFactory
	{
	private:
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;

		VkCommandPool commandPool;
		VkQueue copyQueue;

	public:
		MemoryFactory();
		~MemoryFactory();

		void setPhysicalDevice( VkPhysicalDevice& physicalDevice ) { this->physicalDevice = physicalDevice; }
		void setLogicalDevice( VkDevice& logicalDevice ) { this->logicalDevice = logicalDevice; }
		void setCommandPool( VkCommandPool& commandPool ) { this->commandPool = commandPool; }
		void setBufferCopyQueue( VkQueue& copyQueue ) { this->copyQueue = copyQueue; }

		void createTextureImage( char * location, VkImage& dstImage, VkDeviceMemory& dstMemory );
		void transitionImageLayout( VkImage & image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout );
		void copyBufferToImage( VkBuffer & buffer, VkImage & image, uint32_t width, uint32_t height );

		void createBufferMemory( VkDeviceSize size, void const * srcData, VkBuffer & dstBuffer, VkDeviceMemory & dstMemory, VkBufferUsageFlagBits flags );
		void createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property, VkBuffer & buffer, VkDeviceMemory & memory );
		void copyBuffer( VkBuffer src, VkBuffer dst, VkDeviceSize size );

		VkCommandBuffer beginOneTimeUsageCommand();
		void endOneTimeUsageCommand( VkCommandBuffer & cmdBuffer );
	};
};