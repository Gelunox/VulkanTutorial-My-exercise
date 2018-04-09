#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "../util/Util.hpp"
#include "../Vertex.hpp"

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class PipelineBuilder
	{
	public:
		typedef PipelineBuilder & This;

	private:
		VkVertexInputBindingDescription bindDescription = Vertex::getBindDescription();
		array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::getAttributeDescriptions();
		
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssInfo = {};
		VkViewport viewport = {};
		VkRect2D scissor = {};
		VkPipelineViewportStateCreateInfo viewportState = {};
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		VkPipelineColorBlendAttachmentState colorblendAttachment = {};
		VkPipelineColorBlendStateCreateInfo colorblending = {};
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		VkGraphicsPipelineCreateInfo pipelineInfo = {};

		VkDevice device;

		vector<VkShaderModule> shaderModules;
		vector<VkPipelineShaderStageCreateInfo> shaderStages;
	public:
		PipelineBuilder(VkDevice & device);
		~PipelineBuilder();

		This addShaderStage( vector<char>& data, const char* name, VkShaderStageFlagBits stage );

		This setImageExtent( VkExtent2D& imageExtent );
		This setPipelineLayout( VkPipelineLayout& layout );
		This setRenderPass( VkRenderPass& renderPass );
		VkPipeline build();

	private:
		VkShaderModule createShaderModule( VkDevice& device, const vector<char>& code );
	};
};