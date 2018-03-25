#include "Util.hpp"

using namespace com::gelunox::vulcanUtils;

vector<char> Util::readFile( const string& filename )
{
	ifstream file( filename, ios::ate | ios::binary );

	if (!file.is_open())
	{
		throw runtime_error( "can't open file" );
	}

	unsigned int fileSize = file.tellg();
	vector<char> buff( fileSize );

	file.seekg( 0 );
	file.read( buff.data(), fileSize );
	file.close();

	return buff;
}

uint32_t Util::findMemoryType( VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties( device, &memProps );

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw runtime_error( "suitable memory type not found" );
}

VkSurfaceCapabilitiesKHR Util::getSurfaceCapabilities( VkPhysicalDevice device, VkSurfaceKHR surface )
{
	VkSurfaceCapabilitiesKHR capabilities;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &capabilities );

	return capabilities;
}

VkExtent2D Util::getExtent( uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR & capabilities )
{
	if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	VkExtent2D extend = { width, height };
	extend.width = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, extend.width ) );
	extend.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, extend.height ) );

	return extend;
}

VkSurfaceFormatKHR Util::getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
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

VkPresentModeKHR Util::getPresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
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