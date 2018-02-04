#include "VulkanWindow.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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