#include "VulkanWindow.hpp"

using namespace com::gelunox::vulcanUtils;
using namespace std;

void VulkanWindow::buildSwapchain()
{
	VkSurfaceCapabilitiesKHR capabilities = getSurfaceCapabilities();
	swapchainExtent = getSwapExtent( capabilities );
	VkPresentModeKHR presentMode = getPresentMode();
	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat();
	swapchainImageFormat = surfaceFormat.format;

	unsigned int imageCount = capabilities.minImageCount + 1;

	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //blend with the system
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE; //old swapchain if recreating because of resizing or whatever

	if (vkCreateSwapchainKHR( device, &createInfo, nullptr, &swapchain ) != VK_SUCCESS)
	{
		throw runtime_error( "Can't initiate swapchain" );
	}
	//need to requery because implementation is allowed to create more than was initially relayed
	vkGetSwapchainImagesKHR( device, swapchain, &imageCount, nullptr );
	swapchainImages.resize( imageCount );
	vkGetSwapchainImagesKHR( device, swapchain, &imageCount, swapchainImages.data() );
}

void VulkanWindow::buildImages()
{
	swapchainImageViews.resize( swapchainImages.size() );

	for (unsigned int i = 0; i < swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView( device, &createInfo, nullptr, &swapchainImageViews[i] ) != VK_SUCCESS)
		{
			throw runtime_error( "Could not create an image view" );
		}
	}
}
