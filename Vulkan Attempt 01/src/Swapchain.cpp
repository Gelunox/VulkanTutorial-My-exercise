#include "Swapchain.hpp"

using namespace com::gelunox::vulcanUtils;
using namespace std;

Swapchain::Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device,
	VkSurfaceKHR surface, QueueIndices queueIndices, VkDescriptorSetLayout descriptorLayout )
	:Swapchain(width, height, physicalDevice, device, surface, queueIndices, descriptorLayout, nullptr)
{

}

Swapchain::Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device,
	VkSurfaceKHR surface, QueueIndices queueIndices, VkDescriptorSetLayout descriptorLayout, Swapchain * oldSwapchain )
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

	pipeline = new GraphicsPipeline( device, extent, imageFormat, descriptorLayout );

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

	swapchain = SwapchainBuilder( device )
		.setSurface( surface )
		.setMinImageCount( imageCount )
		.setSurfaceFormat( surfaceFormat )
		.setImageExtent( extent )
		.addQueueFamily( queueIndices.asList() )
		.setSurfaceCapabilities( capabilities )
		.setPresentMode( presentMode )
		.setOldSwapchain( oldSwapchain )
		.build();

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
		imageViews[i] = ImageViewBuilder(device)
			.setImage( images[i] )
			.setFormat( imageFormat )
			.build();
	}
}

void Swapchain::createFrameBuffers()
{
	frameBuffers.resize( imageViews.size() );

	VkRenderPass renderPass = pipeline->getRenderPass();

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		frameBuffers[i] = FramebufferBuilder( device )
			.addAttachment( imageViews[i] )
			.setRenderPass( renderPass )
			.setExtent( extent )
			.build();
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