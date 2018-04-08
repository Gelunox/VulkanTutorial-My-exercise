#include "VulkanWindow.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "util/Util.hpp"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <thread>
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

	cerr << "validation layer: " << msg << endl
		<< " | flags: " << flags << endl
		<< " | objType: " << objType << endl
		<< " | obj: " << obj << endl
		<< " | location: " << location << endl
		<< " | code: " << code << endl
		<< " | layerPrefix: " << layerPrefix << endl
		<< " | userData: " << userData << endl;

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

	//Vulkan init
	InstanceBuilder builder = InstanceBuilder()
		.setApplicationName( "Hello Triangle" )
		.setEngineName( "White Dragon" )
		.addExtensions( vector<const char*>( glfwExtensions, glfwExtensions + glfwExtensionCount ) )
		.setValidationLayersEnabled( enableValidationLayers );

	if (enableValidationLayers)
	{
		builder.addExtension( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	}

	instance = builder.build();

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

	createDescriptorSetLayout();

	swapchain = new Swapchain( width, height, physicalDevice, logicalDevice, surface, queueIndices, descriptorSetLayout );

	createCommandpool();
	createBuffers();

	createDescriptorPool();
	createDescriptorSet();

	createCommandbuffers();
	createSemaphores();
}

VulkanWindow::~VulkanWindow()
{
	vkDeviceWaitIdle( logicalDevice );

	vkDestroySemaphore( logicalDevice, imageAvailableSemaphore, nullptr );
	vkDestroySemaphore( logicalDevice, renderFinishedSemaphore, nullptr );

	vkDestroyDescriptorSetLayout( logicalDevice, descriptorSetLayout, nullptr );
	vkDestroyDescriptorPool( logicalDevice, descriptorPool, nullptr );

	delete swapchain;

	vkDestroyBuffer( logicalDevice, indexBuffer, nullptr );
	vkFreeMemory( logicalDevice, indexMemory, nullptr );

	vkDestroyBuffer( logicalDevice, vertexBuffer, nullptr );
	vkFreeMemory( logicalDevice, vertexMemory, nullptr );

	vkDestroyBuffer( logicalDevice, uniformBuffer, nullptr );
	vkFreeMemory( logicalDevice, uniformMemory, nullptr );

	vkDestroyImage( logicalDevice, textureImage, nullptr );
	vkFreeMemory( logicalDevice, textureImageMemory, nullptr );

	vkDestroyCommandPool( logicalDevice, commandpool, nullptr );

	vkDestroyDevice( logicalDevice, nullptr );
	vkDestroySurfaceKHR( instance, surface, nullptr );
	DestroyDebugReportCallbackEXT( instance, callback, nullptr );
	vkDestroyInstance( instance, nullptr );
	glfwDestroyWindow( window );
	glfwTerminate();
}

void VulkanWindow::run()
{
	while (!glfwWindowShouldClose( window ))
	{
		glfwPollEvents();
		update();
		drawFrame();
		this_thread::sleep_for( chrono::milliseconds( 10 ) );
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
			queueIndices.graphics = i;
		}

		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, surface, &presentationSupport );
		if (presentationSupport)
		{
			queueIndices.presentation = i;
		}

		if ( queueIndices.isComplete() )
		{
			return;
		}

		i++;
	}
}

void VulkanWindow::recreateSwapchain()
{
	Swapchain * old = swapchain;

	vkDeviceWaitIdle( logicalDevice );
	swapchain = new Swapchain( width, height, physicalDevice, logicalDevice, surface, queueIndices, descriptorSetLayout, old );
	vkFreeCommandBuffers( logicalDevice, commandpool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data() );
	createCommandbuffers();
	delete old;
}