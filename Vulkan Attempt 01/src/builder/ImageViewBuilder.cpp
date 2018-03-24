#include "ImageViewBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef ImageViewBuilder::This This;

ImageViewBuilder::ImageViewBuilder( VkDevice& device ) : device( device )
{
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
}

This ImageViewBuilder::setImage( VkImage & image )
{
	createInfo.image = image;

	return *this;
}

This ImageViewBuilder::setFormat( VkFormat & format )
{
	createInfo.format = format;

	return *this;
}

VkImageView ImageViewBuilder::build()
{
	VkImageView imageView;

	if (vkCreateImageView( device, &createInfo, nullptr, &imageView ) != VK_SUCCESS)
	{
		throw runtime_error( "Could not create an image view" );
	}

	return imageView;
}
