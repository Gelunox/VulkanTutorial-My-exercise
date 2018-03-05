#include "VulkanWindow.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "util.hpp"

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

	errln( "validation layer: " , msg );

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

	swapchain = new Swapchain( width, height, physicalDevice, logicalDevice, surface, queueIndices );

	createCommandpool();

	createBuffers();

	createCommandbuffers();
	createSemaphores();
}

VulkanWindow::~VulkanWindow()
{
	vkDeviceWaitIdle( logicalDevice );

	vkDestroySemaphore( logicalDevice, imageAvailableSemaphore, nullptr );
	vkDestroySemaphore( logicalDevice, renderFinishedSemaphore, nullptr );

	vkDestroyCommandPool( logicalDevice, commandpool, nullptr );

	delete swapchain;

	vkDestroyBuffer( logicalDevice, indexBuffer, nullptr );
	vkFreeMemory( logicalDevice, indexMemory, nullptr );
	vkDestroyBuffer( logicalDevice, vertexBuffer, nullptr );
	vkFreeMemory( logicalDevice, vertexMemory, nullptr );

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

uint32_t VulkanWindow::findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProps );

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw runtime_error( "suitable memory type not found" );
}

void VulkanWindow::recreateSwapchain()
{
	Swapchain * old = swapchain;

	vkDeviceWaitIdle( logicalDevice );
	swapchain = new Swapchain( width, height, physicalDevice, logicalDevice, surface, queueIndices, old );
	vkFreeCommandBuffers( logicalDevice, commandpool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data() );
	createCommandbuffers();
	delete old;
}