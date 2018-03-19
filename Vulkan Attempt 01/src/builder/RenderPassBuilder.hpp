#pragma once
#include <vulkan/vulkan.h>

namespace com::gelunox::vulcanUtils
{
	class RenderPassBuilder
	{
	public:
		typedef RenderPassBuilder & This;

	private:

		VkDevice device;
		VkAttachmentDescription colorAttachment = {};
		VkAttachmentReference colorAttachmentReference = {};
		VkSubpassDescription subpass = {};
		VkSubpassDependency dependency = {};
		VkRenderPassCreateInfo renderPassInfo = {};

	public:
		RenderPassBuilder( VkDevice device );
		~RenderPassBuilder();

		This setImageFormat( VkFormat& imageFormat );

		VkRenderPass build();
	private:
	};
}