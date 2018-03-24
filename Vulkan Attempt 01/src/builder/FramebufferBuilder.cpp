#include "FramebufferBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef FramebufferBuilder::This This;

FramebufferBuilder::FramebufferBuilder(const VkDevice& device ): device(device)
{
	framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffInfo.layers = 1;
}

This FramebufferBuilder::addAttachment( VkImageView& imageView )
{
	attachments.push_back( imageView );

	framebuffInfo.attachmentCount = attachments.size();
	framebuffInfo.pAttachments = attachments.data();

	return *this;
}

This FramebufferBuilder::setRenderPass( VkRenderPass& renderPass )
{
	framebuffInfo.renderPass = renderPass;

	return *this;
}

This FramebufferBuilder::setExtent( VkExtent2D extent )
{
	framebuffInfo.width = extent.width;
	framebuffInfo.height = extent.height;

	return *this;
}

VkFramebuffer FramebufferBuilder::build()
{
	VkFramebuffer buffer;

	if (vkCreateFramebuffer( device, &framebuffInfo, nullptr, &buffer ) != VK_SUCCESS)
	{
		throw runtime_error( "framebuffer could not be created" );
	}

	return buffer;
}