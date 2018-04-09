#pragma once

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class ImageViewBuilder
	{
	public:
		typedef ImageViewBuilder& This;
	private:
		VkImageViewCreateInfo createInfo = {};

		VkDevice device;

		vector<VkImage> images;
	public:
		ImageViewBuilder( VkDevice& device );

		This setImage( VkImage& image );
		This setFormat( VkFormat format );

		VkImageView build();
	};

};