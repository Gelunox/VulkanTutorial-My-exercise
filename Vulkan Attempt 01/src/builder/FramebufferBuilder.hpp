#pragma once

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class FramebufferBuilder
	{
	public:
		typedef FramebufferBuilder& This;
	private:
		VkFramebufferCreateInfo framebuffInfo = {};

		VkDevice device;

		vector<VkImageView> attachments;
	public:
		FramebufferBuilder(const VkDevice& device);

		This addAttachment( VkImageView& imageView );
		This setRenderPass( VkRenderPass & renderPass );
		This setExtent( VkExtent2D extent );

		VkFramebuffer build();
	};
};