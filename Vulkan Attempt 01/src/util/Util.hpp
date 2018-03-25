#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

namespace com::gelunox::vulcanUtils::Util
{
	vector<char> readFile( const string& filename );

	uint32_t findMemoryType( VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties );
	VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice device, VkSurfaceKHR surface );
	VkExtent2D getExtent( uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR & capabilities );
	VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
	VkPresentModeKHR getPresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
}