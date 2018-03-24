#include "PipelineLayoutBuilder.hpp"

using namespace com::gelunox::vulcanUtils;
using namespace std;

typedef PipelineLayoutBuilder::This This;

PipelineLayoutBuilder::PipelineLayoutBuilder( VkDevice & device) : device(device)
{
	//pipeline layout
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
}

This PipelineLayoutBuilder::addDescriptorSetLayout( VkDescriptorSetLayout & layout )
{
	descriptorSetLayouts.push_back( layout );

	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	return *this;
}

VkPipelineLayout PipelineLayoutBuilder::build()
{
	VkPipelineLayout layout;
	if (vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &layout ) != VK_SUCCESS)
	{
		throw runtime_error( "pipeline layout could not be created" );
	}

	return layout;
}
