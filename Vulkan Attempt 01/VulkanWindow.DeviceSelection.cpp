#include "VulkanWindow.hpp"
#include <iostream>

using namespace com::gelunox::vulcanUtils;
using namespace std;

//https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues

void VulkanWindow::selectPhysicalDevice()
{
	uint32_t deviceCount = 0;
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
