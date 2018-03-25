#pragma once

#include <vulkan/vulkan.h>
#include <set>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class SwapchainBuilder
	{
	public:
		typedef SwapchainBuilder& This;
	private:
		VkSwapchainCreateInfoKHR createInfo = {};

		VkDevice device;

		set<uint32_t> queueFamilyIndices;
	public:
		SwapchainBuilder(VkDevice& device);

		This setSurface( VkSurfaceKHR& surface );
		This setMinImageCount( uint32_t& imageCount );
		This setSurfaceFormat( VkSurfaceFormatKHR& format );
		This setImageExtent( VkExtent2D& extent );
		This addQueueFamily( uint32_t& index );
		This addQueueFamilies( vector<uint32_t>& indices );
		This setSurfaceCapabilities( VkSurfaceCapabilitiesKHR& capabilities );
		This setPresentMode( VkPresentModeKHR& presentMode );
		This setOldSwapchain( VkSwapchainKHR & old );
		
		VkSwapchainKHR build();
	};
};