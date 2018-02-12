#include "Pipeline.hpp"

#include "Swapchain.hpp"

using namespace std;
using namespace com::gelunox::vulcanUtils;


Pipeline::Pipeline( VkDevice device, VkExtent2D imageExtent, VkFormat imageFormat )
	:device(device)
{
	createRenderpass( imageFormat );
	createPipeline( imageExtent );
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline( device, graphics, nullptr );
	vkDestroyPipelineLayout( device, layout, nullptr );
	vkDestroyRenderPass( device, renderPass, nullptr );
}

void Pipeline::createRenderpass( VkFormat imageFormat )
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachRef = {};
	colorAttachRef.attachment = 0;
	colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass( device, &renderPassInfo, nullptr, &renderPass ) != VK_SUCCESS)
	{
		throw std::runtime_error( "failed to create render pass!" );
	}
}

void Pipeline::createPipeline( VkExtent2D imageExtent )
{
	//shaders
	vector<char> vertShader = readFile( "shaders/vert.spv" );
	vector<char> fragShader = readFile( "shaders/frag.spv" );

	VkShaderModule vertShaderModule = createShaderModule( device, vertShader );
	VkShaderModule fragShaderModule = createShaderModule( device, fragShader );

	VkPipelineShaderStageCreateInfo vertShStageInfo = {};
	vertShStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShStageInfo.module = vertShaderModule;
	vertShStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShStageInfo = {};
	fragShStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShStageInfo.module = fragShaderModule;
	fragShStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShStageInfo, fragShStageInfo };

	//vertices
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	//input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssInfo = {};
	inputAssInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssInfo.primitiveRestartEnable = VK_FALSE;

	//viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)imageExtent.width;
	viewport.height = (float)imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 0.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = imageExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	//rasterizer.depthBiasConstantFactor = 0.0f;
	//rasterizer.depthBiasClamp = 0.0f;
	//rasterizer.depthBiasSlopeFactor = 0.0f;

	//multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//multisampling.minSampleShading = 1.0f;
	//multisampling.pSampleMask = nullptr;
	//multisampling.alphaToCoverageEnable = VK_FALSE;
	//multisampling.alphaToOneEnable = VK_FALSE;

	//color blending
	VkPipelineColorBlendAttachmentState colorblendAttachment = {};
	colorblendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorblendAttachment.blendEnable = VK_FALSE;
	//colorblendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorblendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorblendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//colorblendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorblendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorblendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorblending = {};
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

	//VkPipelineDynamicStateCreateInfo dynamicState = {};
	//dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicState.dynamicStateCount = 2;
	//dynamicState.pDynamicStates = dynamicStates;

	//pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &layout ) != VK_SUCCESS)
	{
		throw runtime_error( "pipeline layout could not be created" );
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorblending;
	//pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics ) != VK_SUCCESS)
	{
		throw runtime_error( "graphics pipeline creation failed" );
	}

	vkDestroyShaderModule( device, vertShaderModule, nullptr );
	vkDestroyShaderModule( device, fragShaderModule, nullptr );
}

VkShaderModule Pipeline::createShaderModule( VkDevice device, const vector<char>& code )
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