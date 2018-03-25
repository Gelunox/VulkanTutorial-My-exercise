#include "SwapchainBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef SwapchainBuilder::This This;

SwapchainBuilder::SwapchainBuilder(VkDevice& device): device(device)
{
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //blend with the system
	createInfo.clipped = VK_TRUE;
}

This SwapchainBuilder::setSurface( VkSurfaceKHR& surface )
{
	createInfo.surface = surface;

	return *this;
}

This SwapchainBuilder::setMinImageCount( uint32_t& imageCount )
{
	createInfo.minImageCount = imageCount;

	return *this;
}

This SwapchainBuilder::setSurfaceFormat( VkSurfaceFormatKHR& format )
{
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;

	return *this;
}

This SwapchainBuilder::setImageExtent(VkExtent2D& extent)
{
	createInfo.imageExtent = extent;

	return *this;
}

This SwapchainBuilder::addQueueFamily( uint32_t& index )
{
	queueFamilyIndices.insert( index );

	return *this;
}

This SwapchainBuilder::addQueueFamilies( vector<uint32_t>& indices )
{
	for(uint32_t index: indices)
		queueFamilyIndices.insert( index );

	return *this;
}

This SwapchainBuilder::setSurfaceCapabilities( VkSurfaceCapabilitiesKHR& capabilities )
{
	createInfo.preTransform = capabilities.currentTransform;

	return *this;
}

This SwapchainBuilder::setPresentMode( VkPresentModeKHR& presentMode )
{
	createInfo.presentMode = presentMode;

	return *this;
}

This SwapchainBuilder::setOldSwapchain(VkSwapchainKHR& old)
{
	createInfo.oldSwapchain = old; //old swapchain if recreating because of resizing or whatever

	return *this;
}

VkSwapchainKHR SwapchainBuilder::build()
{
	vector<uint32_t> indexList;
	copy( queueFamilyIndices.begin(), queueFamilyIndices.end(), back_inserter( indexList ) );

	if (indexList.size() == 1)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = indexList.size();
		createInfo.pQueueFamilyIndices = indexList.data();
	}

	VkSwapchainKHR swapchain;
	if (vkCreateSwapchainKHR( device, &createInfo, nullptr, &swapchain ) != VK_SUCCESS)
	{
		throw runtime_error( "Can't initiate swapchain" );
	}

	return swapchain;
}