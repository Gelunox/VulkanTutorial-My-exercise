#include "Swapchain.hpp"

using namespace com::gelunox::vulcanUtils;
using namespace std;

Swapchain::Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueIndices queueIndices )
	:Swapchain(width, height, physicalDevice, device, surface, queueIndices, nullptr)
{

}

Swapchain::Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueIndices queueIndices, Swapchain * oldSwapchain )
	: device( device ), width( width ), height( height )
{
	if (oldSwapchain)
	{
		createSwapchain( physicalDevice, device, surface, queueIndices, oldSwapchain->getSwapchain() );
	}
	else
	{
		createSwapchain( physicalDevice, device, surface, queueIndices, VK_NULL_HANDLE );
	}
	createImages( device );

	pipeline = new Pipeline( device, extent, imageFormat );

	createFrameBuffers();
}

Swapchain::~Swapchain()
{
	delete pipeline;

	for (VkFramebuffer framebuff : frameBuffers)
	{
		vkDestroyFramebuffer( device, framebuff, nullptr );
	}

	for (VkImageView image : imageViews)
	{
		vkDestroyImageView( device, image, nullptr );
	}
	vkDestroySwapchainKHR( device, swapchain, nullptr );
}

void Swapchain::createSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueIndices queueIndices, VkSwapchainKHR oldSwapchain )
{
	VkSurfaceCapabilitiesKHR capabilities = getSurfaceCapabilities( physicalDevice, surface );
	extent = getExtent( width, height, capabilities );
	VkPresentModeKHR presentMode = getPresentMode(physicalDevice, surface);
	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat( physicalDevice, surface);
	imageFormat = surfaceFormat.format;

	uint32_t imageCount = capabilities.minImageCount + 1;

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
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto indexList = queueIndices.asList();

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

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //blend with the system
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapchain; //old swapchain if recreating because of resizing or whatever

	if (vkCreateSwapchainKHR( device, &createInfo, nullptr, &swapchain ) != VK_SUCCESS)
	{
		throw runtime_error( "Can't initiate swapchain" );
	}
	//need to requery because implementation is allowed to create more than was initially relayed
	vkGetSwapchainImagesKHR( device, swapchain, &imageCount, nullptr );
	images.resize( imageCount );
	vkGetSwapchainImagesKHR( device, swapchain, &imageCount, images.data() );
}

void Swapchain::createImages( VkDevice device )
{
	imageViews.resize( images.size() );

	for (size_t i = 0; i < images.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView( device, &createInfo, nullptr, &imageViews[i] ) != VK_SUCCESS)
		{
			throw runtime_error( "Could not create an image view" );
		}
	}
}

void Swapchain::createFrameBuffers()
{
	frameBuffers.resize( imageViews.size() );

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		VkImageView attachments[] = { imageViews[i] };
		VkFramebufferCreateInfo framebuffInfo = {};
		framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffInfo.renderPass = pipeline->getRenderPass();
		framebuffInfo.attachmentCount = 1;
		framebuffInfo.pAttachments = attachments;
		framebuffInfo.width = extent.width;
		framebuffInfo.height = extent.height;
		framebuffInfo.layers = 1;

		if (vkCreateFramebuffer( device, &framebuffInfo, nullptr, &frameBuffers[i] ) != VK_SUCCESS)
		{
			throw runtime_error( "framebuffer could not be created" );
		}
	}
}

VkSurfaceCapabilitiesKHR Swapchain::getSurfaceCapabilities( VkPhysicalDevice device, VkSurfaceKHR surface )
{
	VkSurfaceCapabilitiesKHR capabilities;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &capabilities );

	return capabilities;
}

VkExtent2D Swapchain::getExtent( uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR & capabilities )
{
	if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	VkExtent2D extend = { width, height };
	extend.width = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, extend.width ) );
	extend.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, extend.height ) );

	return extend;
}

VkSurfaceFormatKHR Swapchain::getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
{
	uint32_t formatCount;
	vector<VkSurfaceFormatKHR> formats;

	vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, nullptr );

	if (formatCount)
	{
		formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, formats.data() );
	}

	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (VkSurfaceFormatKHR format : formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR Swapchain::getPresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
{
	uint32_t modeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &modeCount, nullptr );

	vector<VkPresentModeKHR> modes;

	if (modeCount)
	{
		modes.resize( modeCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &modeCount, modes.data() );
	}

	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;

	for (VkPresentModeKHR presentMode : modes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentMode;
		}
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			mode = presentMode;
		}
	}

	return mode;
}