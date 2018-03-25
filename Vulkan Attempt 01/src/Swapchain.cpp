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
	VkSurfaceCapabilitiesKHR capabilities = Util::getSurfaceCapabilities( physicalDevice, surface );
	extent = Util::getExtent( width, height, capabilities );
	VkPresentModeKHR presentMode = Util::getPresentMode(physicalDevice, surface);
	VkSurfaceFormatKHR surfaceFormat = Util::getSurfaceFormat( physicalDevice, surface);
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
		.addQueueFamilies( queueIndices.asList() )
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