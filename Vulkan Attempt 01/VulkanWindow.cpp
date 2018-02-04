#include "VulkanWindow.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <set>
#include <algorithm>

using namespace com::gelunox::vulcanUtils;
using namespace std;

bool VulkanWindow::isSuitableGpu( VkPhysicalDevice device )
{
	VkPhysicalDeviceProperties deviceProps;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties( device, &deviceProps );
	vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

	//vkGetPhysicalDeviceSurfaceSupportKHR

	return deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		&& deviceFeatures.geometryShader;
}

VulkanWindow::VulkanWindow()
{
	//GLFW init
	glfwInit();

	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	window = glfwCreateWindow( 500, 500, "Vulkan window", nullptr, nullptr );

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

	//Vulkan init
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "White Dragon";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
	instanceCreateInfo.enabledLayerCount = 0;

	if (vkCreateInstance( &instanceCreateInfo, nullptr, &instance ) != VK_SUCCESS)
	{
		throw runtime_error( "can't create vulkan instance" );
	}

	//Window surface
	if (glfwCreateWindowSurface( instance, window, nullptr, &surface ) != VK_SUCCESS)
	{
		throw runtime_error( "could not create window surface" );
	}

	selectPhysicalDevice();
	createLogicalDevice();
	buildSwapchain();
}

VulkanWindow::~VulkanWindow()
{
	for (VkImageView image : swapchainImageViews)
	{
		vkDestroyImageView( device, image, nullptr );
	}
	vkDestroySwapchainKHR( device, swapchain, nullptr );
	vkDestroyDevice( device, nullptr );
	vkDestroySurfaceKHR( instance, surface, nullptr );
	vkDestroyInstance( instance, nullptr );
	glfwDestroyWindow( window );
	glfwTerminate();
}

void VulkanWindow::run()
{
	while (!glfwWindowShouldClose( window ))
	{
		glfwPollEvents();
	}
}

void VulkanWindow::selectPhysicalDevice()
{
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );

	if (deviceCount == 0)
	{
		throw runtime_error( "No GPUs with vulkan support" );
	}

	vector<VkPhysicalDevice> devices( deviceCount );
	vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() );

	for (auto& device : devices)
	{
		VkPhysicalDeviceProperties deviceProps;
		vkGetPhysicalDeviceProperties( device, &deviceProps );

		cout << deviceProps.deviceName << endl;
	}
	cout << endl;

	physicalDevice = VK_NULL_HANDLE;

	//Check device suitability
	for (auto& device : devices)
	{
		if (isSuitableGpu( device ))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw runtime_error( "No suitable gpu was found" );
	}
}

void VulkanWindow::createLogicalDevice()
{
	//Logical device creation
	findQFamilyIndexes();
	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = graphicsQIndex;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceQueueCreateInfo presentationQueueCreateInfo = {};
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = presentationQIndex;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.push_back( graphicsQueueCreateInfo );
	queueCreateInfos.push_back( presentationQueueCreateInfo );

	//used device features
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.enabledLayerCount = 0;

	if (VK_SUCCESS != vkCreateDevice( physicalDevice, &deviceCreateInfo, nullptr, &device ))
	{
		throw runtime_error( "cannot create logical device" );
	}

	//retrieve queue handle
	VkQueue graphicsQueue;
	VkQueue presentQ;
	vkGetDeviceQueue( device, graphicsQIndex, 0, &graphicsQueue );
	vkGetDeviceQueue( device, presentationQIndex, 0, &presentQ );
}

//retrieve graphics query and presentation query
void VulkanWindow::findQFamilyIndexes()
{
	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );

	vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );

	int i = 1; //example retrieves last queue?
	for (auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQIndex = i;
		}

		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, surface, &presentationSupport );
		if (presentationSupport)
		{
			presentationQIndex = i;
		}

		if (graphicsQIndex > -1 && presentationQIndex > -1)
		{
			return;
		}

		i++;
	}
}

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

VkSurfaceCapabilitiesKHR VulkanWindow::getSurfaceCapabilities()
{
	VkSurfaceCapabilitiesKHR capabilities;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &capabilities );

	return capabilities;
}

VkExtent2D VulkanWindow::getSwapExtent( VkSurfaceCapabilitiesKHR& capabilities )
{
	if (capabilities.currentExtent.width != numeric_limits<unsigned int>::max())
	{
		return capabilities.currentExtent;
	}

	VkExtent2D extend = { 500, 500 };
	extend.width = max( capabilities.minImageExtent.width, min( capabilities.maxImageExtent.width, extend.width ) );
	extend.height = max( capabilities.minImageExtent.height, min( capabilities.maxImageExtent.height, extend.height ) );

	return extend;
}

VkSurfaceFormatKHR VulkanWindow::getSurfaceFormat()
{
	unsigned int formatCount;
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

VkPresentModeKHR VulkanWindow::getPresentMode()
{
	unsigned int modeCount;
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