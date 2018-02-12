#include "VulkanWindow.hpp"

#include <iostream>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <thread>
#include <chrono>
#include <set>
#include <algorithm>
#include <iostream>

using namespace com::gelunox::vulcanUtils;
using namespace std;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData ) {

	cerr << "validation layer: " << msg << endl;

	return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT( VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback )
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
	if (func != nullptr)
	{
		return func( instance, pCreateInfo, pAllocator, pCallback );
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator )
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	if (func != nullptr)
	{
		func( instance, callback, pAllocator );
	}
}

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

//https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
VulkanWindow::VulkanWindow()
{
	//GLFW init
	glfwInit();

	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	window = glfwCreateWindow( width, height, "Vulkan window", nullptr, nullptr );

	glfwSetWindowUserPointer( window, this );
	glfwSetWindowSizeCallback( window, VulkanWindow::onWindowResized );

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
	std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );
	if (enableValidationLayers)
	{
		extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	}


	//Vulkan init
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "White Dragon";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
	createInfo.ppEnabledExtensionNames = extensions.data();
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS)
	{
		throw runtime_error( "can't create vulkan instance" );
	}

	VkDebugReportCallbackCreateInfoEXT debugInfo = {};
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugInfo.pfnCallback = debugCallback;

	CreateDebugReportCallbackEXT( instance, &debugInfo, nullptr, &callback );
	
	//Window surface
	if (glfwCreateWindowSurface( instance, window, nullptr, &surface ) != VK_SUCCESS)
	{
		throw runtime_error( "could not create window surface" );
	}

	selectPhysicalDevice();
	findQFamilyIndexes();
	createLogicalDevice();

	createSwapchain();
	createImages();

	createRenderPass();
	createGraphicsPipeline();

	createFramebuffers();
	createCommandpool();
	createCommandbuffers();
	createSemaphores();
}

VulkanWindow::~VulkanWindow()
{
	vkDeviceWaitIdle( logicalDevice );

	vkDestroySemaphore( logicalDevice, renderFinishedSemaphore, nullptr );
	vkDestroySemaphore( logicalDevice, renderFinishedSemaphore, nullptr );
	vkDestroyCommandPool( logicalDevice, commandpool, nullptr );

	cleanupSwapchain();

	vkDestroyDevice( logicalDevice, nullptr );
	vkDestroySurfaceKHR( instance, surface, nullptr );
	DestroyDebugReportCallbackEXT( instance, callback, nullptr );
	vkDestroyInstance( instance, nullptr );
	glfwDestroyWindow( window );
	glfwTerminate();


	char c;
	std::cin >> c;
}

void VulkanWindow::run()
{
	while (!glfwWindowShouldClose( window ))
	{
		glfwPollEvents();
		drawFrame();
		this_thread::sleep_for( chrono::milliseconds( 33 ) );
	}
}

void VulkanWindow::onWindowResized( int width, int height )
{
	if (width == 0 || height == 0)
	{
		return;
	}
	this->width = width;
	this->height = height;

	recreateSwapchain();
}

void VulkanWindow::onWindowResized( GLFWwindow * window, int width, int height )
{
	VulkanWindow * app = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer( window ));
	app->onWindowResized( width, height );
}

//https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
//retrieve graphics query and presentation query
void VulkanWindow::findQFamilyIndexes()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );

	vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );

	int i = 0; //example retrieves last queue?
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

		if (graphicsQIndex >= 0 && presentationQIndex >= 0)
		{
			return;
		}

		i++;
	}
}

//https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface

VkSurfaceCapabilitiesKHR VulkanWindow::getSurfaceCapabilities()
{
	VkSurfaceCapabilitiesKHR capabilities;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &capabilities );

	return capabilities;
}

VkExtent2D VulkanWindow::getSwapExtent( VkSurfaceCapabilitiesKHR& capabilities )
{
	if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	VkExtent2D extend = { width, height };
	extend.width = max( capabilities.minImageExtent.width, min( capabilities.maxImageExtent.width, extend.width ) );
	extend.height = max( capabilities.minImageExtent.height, min( capabilities.maxImageExtent.height, extend.height ) );

	return extend;
}

VkSurfaceFormatKHR VulkanWindow::getSurfaceFormat()
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

VkPresentModeKHR VulkanWindow::getPresentMode()
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

VkShaderModule VulkanWindow::createShaderModule( const vector<char>& code )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;

	if (vkCreateShaderModule( logicalDevice, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS)
	{
		throw runtime_error( "Can't create shader module" );
	}

	return shaderModule;
}