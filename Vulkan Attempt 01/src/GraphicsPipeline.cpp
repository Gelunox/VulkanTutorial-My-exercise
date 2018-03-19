#include "GraphicsPipeline.hpp"

using namespace std;
using namespace com::gelunox::vulcanUtils;


GraphicsPipeline::GraphicsPipeline( VkDevice device, VkExtent2D imageExtent, VkFormat imageFormat, VkDescriptorSetLayout descriptorLayout )
	:device(device)
{
	createRenderpass( imageFormat );
	createPipeline( imageExtent, descriptorLayout );
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline( device, graphics, nullptr );
	vkDestroyPipelineLayout( device, layout, nullptr );
	vkDestroyRenderPass( device, renderPass, nullptr );
}
void GraphicsPipeline::createRenderpass( VkFormat imageFormat )
{
	renderPass = RenderPassBuilder( device )
		.setImageFormat( imageFormat )
		.build();
}

void GraphicsPipeline::createPipeline( VkExtent2D imageExtent, VkDescriptorSetLayout descriptorLayout )
{
	//shaders
	vector<char> vertShader = readFile( "shaders/vert.spv" );
	vector<char> fragShader = readFile( "shaders/frag.spv" );

	layout = PipelineLayoutBuilder( device )
		.addDescriptorSetLayout( descriptorLayout )
		.build();

	graphics = PipelineBuilder( device )
		.addShaderStage( vertShader, "main", VK_SHADER_STAGE_VERTEX_BIT )
		.addShaderStage( fragShader, "main", VK_SHADER_STAGE_FRAGMENT_BIT )
		.setImageExtent( imageExtent )
		.setRenderPass( renderPass )
		.setPipelineLayout( layout )
		.build();
}