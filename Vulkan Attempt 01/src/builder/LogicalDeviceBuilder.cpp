#include "LogicalDeviceBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef LogicalDeviceBuilder::This This;

LogicalDeviceBuilder::LogicalDeviceBuilder( VkPhysicalDevice& device ) :device( device )
{
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
}

This LogicalDeviceBuilder::addQueueInfo( VkDeviceQueueCreateInfo& info )
{
	queueCreateInfos.push_back( info );

	return *this;
}

This LogicalDeviceBuilder::addExtension( const char* extension )
{
	deviceExtensions.push_back( extension );

	return *this;
}

This LogicalDeviceBuilder::addExtensions( vector<const char*> extensions )
{
	deviceExtensions.insert( deviceExtensions.end(), extensions.begin(), extensions.end() );

	return *this;
}

This LogicalDeviceBuilder::setValidationLayersEnabled( bool enabled )
{
	if (enabled)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size());
		deviceCreateInfo.ppEnabledLayerNames = VULKAN_VALIDATION_LAYERS.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
	}

	return *this;
}

VkDevice LogicalDeviceBuilder::build()
{
	//Queue createInfos
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	//device extensions
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VkDevice logicalDevice;

	if (VK_SUCCESS != vkCreateDevice( device, &deviceCreateInfo, nullptr, &logicalDevice ))
	{
		throw runtime_error( "cannot create logical device" );
	}

	return logicalDevice;
}