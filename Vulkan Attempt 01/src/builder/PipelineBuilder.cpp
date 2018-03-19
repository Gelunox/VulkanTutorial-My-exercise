#include "PipelineBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef PipelineBuilder::This This;

PipelineBuilder::PipelineBuilder( VkDevice & device ) : device( device )
{
	//vertices
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//input assembly
	inputAssInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssInfo.primitiveRestartEnable = VK_FALSE;

	//viewport
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 0.0f;

	scissor.offset = { 0, 0 };

	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//rasterizer
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	//rasterizer.depthBiasConstantFactor = 0.0f;
	//rasterizer.depthBiasClamp = 0.0f;
	//rasterizer.depthBiasSlopeFactor = 0.0f;

	//multisampling
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//multisampling.minSampleShading = 1.0f;
	//multisampling.pSampleMask = nullptr;
	//multisampling.alphaToCoverageEnable = VK_FALSE;
	//multisampling.alphaToOneEnable = VK_FALSE;

	//color blending
	colorblendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorblendAttachment.blendEnable = VK_FALSE;
	//colorblendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorblendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorblendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//colorblendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorblendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorblendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	colorblending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorblending.logicOpEnable = VK_FALSE;
	colorblending.logicOp = VK_LOGIC_OP_COPY;
	colorblending.attachmentCount = 1;
	colorblending.pAttachments = &colorblendAttachment;
	colorblending.blendConstants[0] = 0.0f;
	colorblending.blendConstants[1] = 0.0f;
	colorblending.blendConstants[2] = 0.0f;
	colorblending.blendConstants[3] = 0.0f;

	//dynamic state
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

	//dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicState.dynamicStateCount = 2;
	//dynamicState.pDynamicStates = dynamicStates;

	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorblending;
	//pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
}

PipelineBuilder::~PipelineBuilder()
{
	for (VkShaderModule module : shaderModules)
	{
		vkDestroyShaderModule( device, module, nullptr );
	}
}

This PipelineBuilder::addShaderStage( vector<char>& data, const char* name, VkShaderStageFlagBits stage )
{
	VkShaderModule module = createShaderModule( device, data );

	VkPipelineShaderStageCreateInfo stageInfo = {};
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage = stage;
	stageInfo.module = module;
	stageInfo.pName = name;

	shaderModules.push_back( module );
	shaderStages.push_back( stageInfo );

	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	return *this;
}

This PipelineBuilder::setImageExtent( VkExtent2D& imageExtent )
{
	viewport.width = (float)imageExtent.width;
	viewport.height = (float)imageExtent.height;

	scissor.extent = imageExtent;

	return *this;
}

This PipelineBuilder::setPipelineLayout( VkPipelineLayout& layout )
{
	pipelineInfo.layout = layout;

	return *this;
}

This PipelineBuilder::setRenderPass( VkRenderPass& renderPass )
{
	pipelineInfo.renderPass = renderPass;

	return *this;
}

VkPipeline PipelineBuilder::build()
{
	VkPipeline pipeline;

	VkResult result = vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline );

	if (result != VK_SUCCESS)
	{
		throw runtime_error( "graphics pipeline creation failed" );
	}

	return pipeline;
}

VkShaderModule PipelineBuilder::createShaderModule( VkDevice& device, const vector<char>& code )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;

	if (vkCreateShaderModule( device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS)
	{
		throw runtime_error( "Can't create shader module" );
	}

	return shaderModule;
}