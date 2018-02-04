#include "VulkanWindow.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "util.hpp"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

using namespace com::gelunox::vulcanUtils;
using namespace std;

//https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics
//https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
//https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions

void com::gelunox::vulcanUtils::VulkanWindow::buildGraphicsPipeline()
{
	vector<char> vertShader = readFile( "shaders/vert.spv" );
	vector<char> fragShader = readFile( "shaders/frag.spv" );

	vertShaderModule = createShaderModule( vertShader );
	fragShaderModule = createShaderModule( fragShader );

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





	vkDestroyShaderModule( device, vertShaderModule, nullptr );
	vkDestroyShaderModule( device, fragShaderModule, nullptr );
}