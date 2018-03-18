#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "util.hpp"
#include "Vertex.hpp"

using namespace std;

namespace com::gelunox::vulcanUtils
{
	//class Swapchain;

	class GraphicsPipeline
	{
	private:
		VkDevice device;

		VkRenderPass renderPass;
		
		VkPipeline graphics;
		VkPipelineLayout layout;

	public:
		GraphicsPipeline(VkDevice device, VkExtent2D imageExtent, VkFormat imageFormat, VkDescriptorSetLayout descriptorLayout);
		~GraphicsPipeline();


		VkRenderPass getRenderPass() { return renderPass; }
		VkPipeline getPipeline() { return graphics; }
		VkPipelineLayout getLayout() { return layout; }

	private:
		void createRenderpass( VkFormat imageFormat );
		void createPipeline( VkExtent2D imageExtent, VkDescriptorSetLayout descriptorLayout );

		static VkShaderModule createShaderModule( VkDevice device, const vector<char>& code );
	};
}