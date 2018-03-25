#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "../util/Globals.hpp"

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class LogicalDeviceBuilder
	{
	public:
		typedef LogicalDeviceBuilder& This;
	private:
		//used device features
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo deviceCreateInfo = {};

		VkPhysicalDevice device;

		vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		vector<const char*> deviceExtensions;
	public:
		LogicalDeviceBuilder(VkPhysicalDevice& device);

		This addQueueInfo( VkDeviceQueueCreateInfo& info );
		This addExtension( const char * extension );
		This addExtensions( vector<const char*> extensions );
		This setValidationLayersEnabled( bool enabled );

		VkDevice build();
	};
};