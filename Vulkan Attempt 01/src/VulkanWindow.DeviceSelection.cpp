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
	memFac.setPhysicalDevice( physicalDevice );
}

void VulkanWindow::createLogicalDevice()
{
	//Logical device creation
	LogicalDeviceBuilder builder = LogicalDeviceBuilder( physicalDevice )
		.addExtensions( deviceExtensions )
		.setValidationLayersEnabled(enableValidationLayers);

	float queuePriority = 1.0f;
	auto indices = queueIndices.asList();

	for (auto index : indices)
	{
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = index;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;

		builder.addQueueInfo( queueInfo );
	}

	logicalDevice = builder.build();

	//retrieve queue handle
	vkGetDeviceQueue( logicalDevice, queueIndices.graphics, 0, &graphicsQ );
	vkGetDeviceQueue( logicalDevice, queueIndices.presentation, 0, &presentQ );

	memFac.setLogicalDevice( logicalDevice );
	memFac.setBufferCopyQueue( graphicsQ );
}
