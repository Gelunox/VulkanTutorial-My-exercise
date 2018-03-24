#pragma once
#include <vulkan/vulkan.h>

namespace com::gelunox::vulcanUtils
{
	class RenderPassBuilder
	{
	public:
		typedef RenderPassBuilder & This;

	private:
		VkAttachmentDescription colorAttachment = {};
		VkAttachmentReference colorAttachmentReference = {};
		VkSubpassDescription subpass = {};
		VkSubpassDependency dependency = {};
		VkRenderPassCreateInfo renderPassInfo = {};

		VkDevice device;
	public:
		RenderPassBuilder( VkDevice device );

		This setImageFormat( VkFormat& imageFormat );

		VkRenderPass build();
	private:
	};
}