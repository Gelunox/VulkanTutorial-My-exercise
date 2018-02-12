#pragma once

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	//class Swapchain;

	class Pipeline
	{
	private:
		VkDevice device;

		VkRenderPass renderPass;

		VkPipeline graphics;
		VkPipelineLayout layout;

	public:
		Pipeline(VkDevice device, VkExtent2D imageExtent, VkFormat imageFormat);
		~Pipeline();

		VkRenderPass getRenderPass() { return renderPass; }
		VkPipeline getPipeline() { return graphics; }

	private:
		void createRenderpass( VkFormat imageFormat );
		void createPipeline( VkExtent2D imageExtent );

		static VkShaderModule createShaderModule( VkDevice device, const vector<char>& code );
	};
}